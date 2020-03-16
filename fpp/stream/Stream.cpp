#include "Stream.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    //base init
    Stream::Stream(AVStream* avstream, SharedParameters parameters)
        : FFmpegObject(avstream)
        , MediaData(parameters->type())
        , params { parameters }
        , _used { false }
        , _stamp_type { StampType::Copy }
        , _prev_dts { 0 }
        , _prev_pts { 0 }
        , _packet_index { 0 }
        , _packet_dts_delta { 0 }
        , _packet_pts_delta { 0 }
        , _packet_duration { 0 }
        , _pts_offset { 0 }
        , _dts_offset { 0 }
        , _start_time_point { FROM_START }
        , _end_time_point { TO_END } {
        setName(utils::to_string(type()) + " stream");
    }

    //input stream
    Stream::Stream(AVStream* avstream)
        : Stream(avstream, utils::make_params(avstream->codecpar->codec_type)) {
        params->parseStream(avstream);
    }

    //output stream
    Stream::Stream(SharedParameters parameters, AVStream* avstream)
        : Stream(avstream, parameters) {
        params->setStreamIndex(avstream->index);
        avstream->time_base = params->timeBase();
        initCodecpar();
    }

    Stream::Stream(SharedParameters params)
        : Stream(nullptr, params) {
    }

//    void Stream::init() { //TODO продумать инициализацию ффмпег потока из параметров по типу completeFrom метода 17.01
////        return_if(inited(), Code::OK);
//        if (inited_ptr(raw())) {
//            /* Инициализация полей параметров кодека значениями из параметров потока */
//            utils::params_to_avcodecpar(params, codecParams());
//        }
//    }

    std::string Stream::toString() const {
        return "[" + std::to_string(params->streamIndex()) + "] "
                + utils::to_string(type()) + " stream: "
                + (used() ? params->toString() : "not used");
    }

    void Stream::stampPacket(Packet& packet) { // TODO refactoring 14.01
        switch (_stamp_type) {
        case StampType::Copy:
            _packet_duration = packet.pts() - _prev_pts;
            break;
        case StampType::Realtime: {
            if (_packet_index == 0) { //TODO костыль сброса таймера на получении первого пакета, перенести в открытие?
                _chronometer.reset_timepoint();
            }

            const auto chronometer_timebase { DEFAULT_TIME_BASE };
            _packet_duration = ::av_rescale_q(_chronometer.elapsed_milliseconds(), chronometer_timebase, params->timeBase());

            _chronometer.reset_timepoint();

            if (_packet_duration < 16) { //TODO костыль: ффмпег отдает первый кадров 10 мгновенно
                const int64_t duration_ms = int64_t(1000 / ::av_q2d(static_cast<VideoParameters*>(params.get())->frameRate()));
                _packet_duration = ::av_rescale_q(duration_ms, DEFAULT_TIME_BASE, params->timeBase());
            }

            if (_packet_index == 0) { //TODO костыль
                packet.setDts(0);
                packet.setPts(0);
                break;
            }

            _packet_dts_delta = _packet_duration;
            _packet_pts_delta = _packet_duration;

            packet.setDts(_prev_dts + _packet_dts_delta);
            packet.setPts(_prev_pts + _packet_pts_delta);
            break;
        }
        case StampType::Rescale: {
            auto debug_value_00 = packet.pts();
            if (packet.isVideo() && (packet.dts() == AV_NOPTS_VALUE)) {
                packet.setDts(0);
                packet.setPts(0);
            }
            /* Рескеил в таймбейс потока без изменений */
            packet.setDts(::av_rescale_q(packet.dts(), packet.timeBase(), params->timeBase()));
            packet.setPts(::av_rescale_q(packet.pts(), packet.timeBase(), params->timeBase()));
            auto debug_value_01 = packet.pts();

            if (packetIndex() == 0) {
                _pts_offset = -packet.pts();
                _dts_offset = -packet.dts();
//                log_error("OFFSET: " << _pts_offset);
            }

            auto debug_value = packet.pts();

            /* Пересчет с учетом смещения */
            auto new_pts = packet.pts() + _pts_offset;
            auto new_dts = packet.dts() + _dts_offset;

            /* Проверка на начало новой последовательности пакетов */
            if (new_pts < _prev_pts) {
                auto offset = params->duration();
                _pts_offset = offset;
                _dts_offset = offset;
                new_pts = packet.pts() + _pts_offset;
                new_dts = packet.dts() + _dts_offset;
                log_error("new_pts < _prev_pts: " << new_pts << " " << offset);
                log_error("source pts: " << debug_value << ", " << debug_value_00 << " -> " << debug_value_01);
            }

            /* Расчет длительности пакета */
            _packet_duration = new_pts - _prev_pts;

            _packet_dts_delta = _packet_duration;
            _packet_pts_delta = _packet_duration;


            /* Установка новых значений */
            packet.setDts(new_dts);
            packet.setPts(new_pts);
            break;
        }
        case StampType::Offset: {
            if (packetIndex() == 0) {
                _pts_offset = -packet.pts();
                _dts_offset = -packet.dts();
            }
            auto new_pts = packet.pts() + _pts_offset;
            auto new_dts = packet.dts() + _dts_offset;
            _packet_duration = new_pts - _prev_pts;
            _packet_dts_delta = _packet_duration;
            _packet_pts_delta = _packet_duration;
            packet.setDts(new_dts);
            packet.setPts(new_pts);
            break;
        }
        }

        packet.setPos(-1);
        packet.setDuration(_packet_duration);
        packet.setTimeBase(params->timeBase());
        params->increaseDuration(packet.duration());
        _prev_dts = packet.dts();
        _prev_pts = packet.pts();
        _packet_index++;
    }

    bool Stream::timeIsOver() const {
        const auto planned_duration { _end_time_point - _start_time_point };
        const auto actual_duration {
            ::av_rescale_q(params->duration(), params->timeBase(), DEFAULT_TIME_BASE)
        };
        if (actual_duration >= planned_duration) {
            log_debug("Time is over: "
                      << utils::time_to_string(actual_duration, DEFAULT_TIME_BASE)
            );
            return true;
        }
        return false;
    }

    void Stream::setUsed(bool value) {
        _used = value;
    }

    void Stream::setStampType(StampType value) {
        _stamp_type = value;
    }

    void Stream::setStartTimePoint(int64_t value) {
        if (_start_time_point == value) {
            return;
        }
        if ((value != FROM_START) && (value < 0)) {
            log_warning("Cannot set start time point less then zero: " << value << ", ignored");
            return;
        }
        if ((_end_time_point != TO_END) && (value > _end_time_point)) {
            log_warning("Cannot set start time point more then end time point "
                        << _end_time_point <<  ": " << value << ", ignored");
            return;
        }
        _start_time_point = value;
    }

    void Stream::setEndTimePoint(int64_t value) {
        if (_end_time_point == value) {
            return;
        }
        if ((value != TO_END) && (value < 0)) {
            log_warning("Cannot set end time point less then zero: " << value << ", ignored");
            return;
        }
        if ((_start_time_point != FROM_START) && (value < _start_time_point)) {
            log_warning("Cannot set end time point less then start time point "
                        << _start_time_point <<  ": " << value << ", ignored");
            return;
        }
        _end_time_point = value;
    }

    int64_t Stream::index() const {
        return raw()->index;
    }

    bool Stream::used() const {
        return _used;
    }

    StampType Stream::stampType() const {
        return _stamp_type;
    }

    int64_t Stream::startTimePoint() const {
        return _start_time_point;
    }

    int64_t Stream::endTimePoint() const {
        return _end_time_point;
    }

    int64_t Stream::packetIndex() const {
        return _packet_index;
    }

    AVCodecParameters* Stream::codecParams() {
        if (not_inited_ptr(raw())) {
            throw std::runtime_error { "stream is nullptr" }; // TODO перенести выброс в метод raw() 04.02
        }
        return raw()->codecpar;
    }

    void Stream::initCodecpar() {
        auto codecpar { raw()->codecpar };
        codecpar->codec_id = params->codecId();
        codecpar->bit_rate = params->bitrate();

        switch (params->type()) {
        case MediaType::Video: {
            const auto video_parameters {
                std::static_pointer_cast<VideoParameters>(params)
            };
            codecpar->codec_type        = AVMediaType::AVMEDIA_TYPE_VIDEO;
            codecpar->width             = int(video_parameters->width());
            codecpar->height            = int(video_parameters->height());
    //        codec->sample_aspect_ratio    = video_parameters->sampl; //TODO
            codecpar->format            = int(video_parameters->pixelFormat());
            break;
        }
        case MediaType::Audio: {
            const auto audio_parameters {
                std::static_pointer_cast<AudioParameters>(params)
            };
            codecpar->codec_type        = AVMediaType::AVMEDIA_TYPE_AUDIO;
            codecpar->channel_layout    = audio_parameters->channelLayout();
            codecpar->channels          = int(audio_parameters->channels());
            codecpar->sample_rate       = int(audio_parameters->sampleRate());
            break;
        }
        default:
            throw std::invalid_argument {
                "Stream::initCodecpar failed becose of bad param's type"
            };
        }
    }

//    SharedStream make_input_stream(const AVStream* avstream) {
//        return std::make_shared<Stream>(avstream);
//    }

} // namespace fpp
