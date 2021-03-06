#include "Stream.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    // base init (private constructor)
    Stream::Stream(AVStream* avstream, Type type)
        : FFmpegObject(avstream)
        , Media(type)
        , _prev_dts { NOPTS_VALUE }
        , _prev_pts { NOPTS_VALUE }
        , _packet_index { 0 }
        , _start_time_point { FROM_START }
        , _end_time_point { TO_END }
        , _stamp_from_zero { false } {
        if (duration() == NOPTS_VALUE) {
            setDuration(0);
        }
    }

    // input stream
    Stream::Stream(AVStream* avstream)
        : Stream(avstream, utils::to_media_type(avstream->codecpar->codec_type)) {
        params = utils::make_params(type());
        params->parseStream(avstream);
    }

    // output stream
    Stream::Stream(AVStream* avstream, const SpParameters parameters)
        : Stream(avstream, parameters->type()) {
        params = parameters;
        params->initCodecpar(codecpar());
    }

    std::string Stream::toString() const {
        const auto tag {
            ::av_dict_get(raw()->metadata, "language", nullptr, AV_DICT_MATCH_CASE)
        };
        const auto lang {
            tag ? '(' + std::string { tag->value } + ')' : ""
        };
        return "[" + std::to_string(index()) + "] "
                + utils::to_string(type()) + " stream" + lang + ": "
                + params->toString();
    }

    void Stream::initCodecpar() {
        params->initCodecpar(codecpar());
    }

    void Stream::stampPacket(Packet& packet) {
        if (packet.timeBase() != DEFAULT_RATIONAL) {
            ::av_packet_rescale_ts(
                packet.ptr()
                , packet.timeBase()
                , params->timeBase()
            );
        }

        if (_stamp_from_zero) {
            shiftStamps(packet);
        } else {
            // TODO: fix duration in MediaInfo (15.04)
        }

        if (packet.duration() == 0) {
            calculatePacketDuration(packet);
        }

//        avoidNegativeTimestamp(packet);
//        checkStampMonotonicity(packet);
//        checkDtsPtsOrder(packet);

        packet.setPos(-1);
        packet.setTimeBase(params->timeBase());

        increaseDuration(packet.duration());
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
                duration()
                , params->timeBase()
                , DEFAULT_TIME_BASE
            )
        };
        if (actual_duration >= planned_duration) {
            log_info()
                << "Time is over: "
                << utils::time_to_string(actual_duration, DEFAULT_TIME_BASE);
            return true;
        }
        return false;
    }

    void Stream::setIndex(int value) {
        raw()->index = value;
    }

    void Stream::setDuration(std::int64_t duration) {
        raw()->duration = duration;
    }

    void Stream::setStartTimePoint(std::int64_t msec) { // TODO (18.05)
        if (_start_time_point == msec) {
            return;
        }
        if ((msec != FROM_START) && (msec < 0)) {
//            log_warning("Cannot set start time point less then zero: ", msec, ", ignored");
            return;
        }
        if ((_end_time_point != TO_END) && (msec > _end_time_point)) {
//            log_warning("Cannot set start time point more then end time point "
//                        , _end_time_point,  ": ", msec, ", ignored");
            return;
        }
        _start_time_point = msec;
    }

    // TODO: use setEndTimePoint for context. do not specify "source.stream(0)->setEndTimePoint(one_minute);"
    void Stream::setEndTimePoint(std::int64_t msec) { // TODO (18.05)
        if (_end_time_point == msec) {
            return;
        }
        if ((msec != TO_END) && (msec < 0)) {
//            log_warning("Cannot set end time point less then zero: ", msec, ", ignored");
            return;
        }
        if ((_start_time_point != FROM_START) && (msec < _start_time_point)) {
//            log_warning("Cannot set end time point less then start time point "
//                        , _start_time_point,  ": ", msec, ", ignored");
            return;
        }
        _end_time_point = msec;
    }

    void Stream::stampFromZero(bool value) {
        _stamp_from_zero = value;
    }

    int Stream::index() const {
        return raw()->index;
    }

    std::int64_t Stream::duration() const {
        return raw()->duration;
    }

    std::int64_t Stream::startTimePoint() const {
        return _start_time_point;
    }

    std::int64_t Stream::endTimePoint() const {
        return _end_time_point;
    }

    std::int64_t Stream::packetIndex() const {
        return _packet_index;
    }

    AVCodecParameters* Stream::codecpar() {
        if (!raw()) {
            throw std::runtime_error { "stream is null" };
        }
        return raw()->codecpar;
    }

    bool Stream::addMetadata(const std::string_view key, const std::string_view value, int flags) {
        ffmpeg_api_non_strict(av_dict_set, &raw()->metadata, key.data(), value.data(), flags);
        return true;
    }

    void Stream::increaseDuration(const std::int64_t value) {
        raw()->duration += value;
    }

    void Stream::shiftStamps(Packet& packet) {
        if (raw()->start_time == NOPTS_VALUE) {
            raw()->start_time = packet.pts();
        }
        if (packet.dts() != NOPTS_VALUE) {
            packet.setDts(packet.dts() - raw()->start_time);
        }
        if (packet.pts() != NOPTS_VALUE) {
            packet.setPts(packet.pts() - raw()->start_time);
        }
    }

    void Stream::calculatePacketDuration(Packet& packet) {
        if (raw()->cur_dts == NOPTS_VALUE) {
            packet.setDuration(0);
        } else {
            packet.setDuration(std::abs(packet.dts() - raw()->cur_dts));
        }
    }

//    void Stream::avoidNegativeTimestamp(Packet& packet) {
//        if (packet.dts() < 0) {
//            packet.setDts(0);
//        }
//        if (packet.pts() < 0) {
//            packet.setPts(0);
//        }
//    }

//    void Stream::checkStampMonotonicity(Packet& packet) {
//        if (_prev_dts == AV_NOPTS_VALUE) {
//            _prev_dts = packet.dts();
//            return;
//        }
//        if (_prev_dts >= packet.dts()) {
//            log_warning(
//                "Application provided invalid, "
//                "non monotonically increasing dts to muxer "
//                "in stream " << packet.streamIndex() << ": "
//                << _prev_dts << " >= " << packet.dts()
//            );
//            packet.setDts(_prev_dts + 1);
//        }
//    }

//    void Stream::checkDtsPtsOrder(Packet& packet) {
//        if (packet.pts() == AV_NOPTS_VALUE) {
//            packet.setPts(packet.dts());
//            return;
//        }
//        if (packet.pts() < packet.dts()) {
//            log_warning(
//                "pts (" << packet.pts() << ") < dts (" << packet.dts() << ") "
//                << "in stream " << packet.streamIndex()
//            );
//            packet.setPts(packet.dts());
//        }
//    }

} // namespace fpp
