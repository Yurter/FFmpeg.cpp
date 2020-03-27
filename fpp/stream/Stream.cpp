#include "Stream.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    // base init (private constructor)
    Stream::Stream(AVStream* avstream, MediaType type)
        : FFmpegObject(avstream)
        , MediaData(type)
        , _prev_dts { AV_NOPTS_VALUE }
        , _prev_pts { AV_NOPTS_VALUE }
        , _packet_index { 0 }
        , _start_time_point { FROM_START }
        , _end_time_point { TO_END } {

        setName("Stream");

    }

    // input stream
    Stream::Stream(AVStream* avstream)
        : Stream(avstream, utils::to_media_type(avstream->codecpar->codec_type)) {

        setName("In" + utils::to_string(type()) + "Stream");
        params = utils::make_params(type());
        params->parseStream(avstream);

    }

    // output stream
    Stream::Stream(AVStream* avstream, const SharedParameters parameters)
        : Stream(avstream, parameters->type()) {

        setName("Out" + utils::to_string(type()) + "Stream");
        params = parameters;
        params->initCodecpar(codecpar());
        if (invalid_int(params->streamIndex())) {
            params->setStreamIndex(index());
        }
        else {
            // index pre-setted for some reason
            raw()->index = int(params->streamIndex());
        }

    }

    std::string Stream::toString() const {
        return "[" + std::to_string(index()) + "] "
                + utils::to_string(type()) + " stream: "
                + params->toString();
    }

    void Stream::stampPacket(Packet& packet) {

        if (packet.timeBase() != DEFAULT_RATIONAL) {
            ::av_packet_rescale_ts(
                packet.ptr()
                , packet.timeBase()
                , params->timeBase()
            );
        }
        else if (raw()->start_time != AV_NOPTS_VALUE) {
            packet.setDts(packet.dts() - raw()->start_time);
            packet.setPts(packet.pts() - raw()->start_time);
        }

        if (packet.duration() == 0) {
            if (raw()->cur_dts == AV_NOPTS_VALUE) {
                const auto fps {
                    ::av_q2intfloat(
                        std::static_pointer_cast<VideoParameters>(params)->frameRate()
                    )
                };
                const auto inv_tb {
                    ::av_q2intfloat(::av_inv_q(params->timeBase()))
                };
                const auto avg_pkt_dur {
                    inv_tb / fps
                };
                packet.setDuration(avg_pkt_dur);
            } else {
                packet.setDuration(std::abs(packet.dts() - raw()->cur_dts));
            }
        }

        checkStampMonotonicity(packet);
        checkDtsPtsOrder(packet);

        packet.setPos(-1);
        packet.setTimeBase(params->timeBase());

        params->increaseDuration(packet.duration());
        _prev_dts = packet.dts();
        _prev_pts = packet.pts();
        _packet_index++;

    }

    bool Stream::timeIsOver() const {
        const auto planned_duration {
            _end_time_point - _start_time_point
        };
        const auto actual_duration {
            ::av_rescale_q(
                params->duration()
                , params->timeBase()
                , DEFAULT_TIME_BASE
            )
        };
        if (actual_duration >= planned_duration) {
            log_debug(
                "Time is over: "
                << utils::time_to_string(actual_duration, DEFAULT_TIME_BASE)
            );
            return true;
        }
        return false;
    }

    void Stream::setIndex(int64_t value) {
        raw()->index = int(value);
        params->setStreamIndex(value);
    }

    void Stream::setStartTimePoint(int64_t msec) {
        if (_start_time_point == msec) {
            return;
        }
        if ((msec != FROM_START) && (msec < 0)) {
            log_warning("Cannot set start time point less then zero: " << msec << ", ignored");
            return;
        }
        if ((_end_time_point != TO_END) && (msec > _end_time_point)) {
            log_warning("Cannot set start time point more then end time point "
                        << _end_time_point <<  ": " << msec << ", ignored");
            return;
        }
        _start_time_point = msec;
    }

    void Stream::setEndTimePoint(int64_t msec) {
        if (_end_time_point == msec) {
            return;
        }
        if ((msec != TO_END) && (msec < 0)) {
            log_warning("Cannot set end time point less then zero: " << msec << ", ignored");
            return;
        }
        if ((_start_time_point != FROM_START) && (msec < _start_time_point)) {
            log_warning("Cannot set end time point less then start time point "
                        << _start_time_point <<  ": " << msec << ", ignored");
            return;
        }
        _end_time_point = msec;
    }

    int64_t Stream::index() const {
        return raw()->index;
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

    AVCodecParameters* Stream::codecpar() {
        if (not_inited_ptr(raw())) {
            throw std::runtime_error { "stream is null" };
        }
        return raw()->codecpar;
    }

    void Stream::checkStampMonotonicity(Packet& packet) {
        if (_prev_dts == AV_NOPTS_VALUE) {
            _prev_dts = packet.dts();
            return;
        }
        if (packet.dts() <= _prev_dts) {
            log_warning(
                "Application provided invalid, "
                "non monotonically increasing dts to muxer "
                "in stream " << packet.streamIndex() << ": "
                << _prev_dts << " >= " << packet.dts()
            );
            packet.setDts(_prev_dts + 1);
        }
    }

    void Stream::checkDtsPtsOrder(Packet& packet) {
        if (packet.pts() < packet.dts()) {
            log_warning(
                "pts (" << packet.pts() << ") < dts (" << packet.dts() << ") "
                << "in stream " << packet.streamIndex()
            );
            packet.setPts(packet.dts());
        }
    }

} // namespace fpp
