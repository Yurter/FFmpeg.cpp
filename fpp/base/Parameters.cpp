#include "Parameters.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

#define DEFAULT_CODEC_ID        AV_CODEC_ID_NONE
#define inited_codec_id(x)      ((x) != DEFAULT_CODEC_ID)
#define not_inited_codec_id(x)  ((x) == DEFAULT_CODEC_ID)

namespace fpp {

    Parameters::Parameters(MediaType type)
        : MediaData(type)
        , _codec { nullptr }
        , _bitrate { 0 }
        , _duration { 0 }
        , _stream_index { INVALID_INT }
        , _time_base { DEFAULT_RATIONAL } {
        setName("Parameters");
    }

    void Parameters::setDecoder(AVCodecID codec_id) {
        setCodec(::avcodec_find_decoder(codec_id));
    }

    void Parameters::setEncoder(AVCodecID codec_id) {
        setCodec(::avcodec_find_encoder(codec_id));
    }

    bool Parameters::isDecoder() const {
        return ::av_codec_is_decoder(codec()) != 0;
    }

    bool Parameters::isEncoder() const {
        return ::av_codec_is_encoder(codec()) != 0;
    }

    void Parameters::setCodec(AVCodec* codec) {
        if (not_inited_ptr(codec)) {
            throw std::runtime_error {
                std::string { __FUNCTION__ } + " failed: codec is null"
            };
        }
        _codec = codec;
    }

    void Parameters::setBitrate(int64_t bitrate) {
        _bitrate = bitrate;
    }

    void Parameters::setDuration(int64_t duration) {
        if (duration < 0) {
            log_warning("Cannot set duration less then zero: " << duration << ", ignored");
            return;
        }
        if (duration > LONG_MAX) {
            log_warning("Cannot set duration more then LONG_MAX, ignored");
            return;
        }
        _duration = duration;
    }

    void Parameters::setStreamIndex(uid_t stream_index) {
        _stream_index = stream_index;
    }

    void Parameters::setTimeBase(AVRational time_base) {
        _time_base = time_base;
    }

    AVCodecID Parameters::codecId() const {
        return not_inited_ptr(_codec) ? DEFAULT_CODEC_ID : _codec->id;
    }

    std::string Parameters::codecName() const {
        return _codec->name;
    }

    AVCodec* Parameters::codec() const {
        if (not_inited_ptr(_codec)) {
            throw std::runtime_error { "codec is null" };
        }
        return _codec;
    }

    int64_t Parameters::bitrate() const {
        return _bitrate;
    }

    int64_t Parameters::duration() const {
         return _duration;
    }

    uid_t Parameters::streamIndex() const {
        return _stream_index;
    }

    AVRational Parameters::timeBase() const {
        return _time_base;
    }

    void Parameters::increaseDuration(const int64_t value) {
        _duration += value;
    }

    std::string Parameters::toString() const {
        return utils::to_string(type()) + " "
            + codecName() + " "
            + (::av_codec_is_decoder(codec()) ? "decoder" : "encoder") + " "
            + std::to_string(bitrate()) + " bit/s, "
            + "dur " + std::to_string(duration()) + ", "
            + "tb " + utils::to_string(timeBase());
    }

    void Parameters::completeFrom(const SharedParameters other) {
        if (not_inited_codec_id(codecId())) { setEncoder(other->codecId());     }
        if (not_inited_int(bitrate()))      { setBitrate(other->bitrate());     }
        if (not_inited_q(timeBase()))       { setTimeBase(other->timeBase());   }
    }

    void Parameters::parseStream(const AVStream* avstream) {
        setDecoder(avstream->codecpar->codec_id);
        setBitrate(avstream->codecpar->bit_rate);
        setDuration(avstream->duration);
        setStreamIndex(avstream->index);
        setTimeBase(avstream->time_base);
    }

    void Parameters::initStream(AVStream* avstream) const {
        avstream->codecpar->codec_id = codecId();
        avstream->codecpar->bit_rate = bitrate();
        avstream->duration = duration();
        avstream->time_base = timeBase();
    }

    bool Parameters::betterThen(const SharedParameters& other) {
        return this->bitrate() > other->bitrate();
    }

} // namespace fpp
