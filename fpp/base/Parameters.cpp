#include "Parameters.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

#define DEFAULT_CODEC_ID        AV_CODEC_ID_NONE
#define not_inited_codec_id(x)  ((x) == DEFAULT_CODEC_ID)

namespace fpp {

    Parameters::Parameters(MediaType type)
        : MediaData(type)
        , _codec { nullptr }
        , _codec_tag { 0 }
        , _bitrate { 0 }
        , _duration { 0 }
        , _stream_index { INVALID_INT }
        , _time_base { DEFAULT_RATIONAL }
        , _bits_per_coded_sample { 0 }
        , _bits_per_raw_sample { 0 }
        , _profile { 0 }
        , _level { 0 } {
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
        _codec = codec;
    }

    void Parameters::setBitrate(int64_t bitrate) {
        _bitrate = bitrate;
    }

    void Parameters::setDuration(int64_t duration) {
        if ((duration - 1) == INT64_MAX) {
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

    void Parameters::setExtradata(Extradata extradata) {
        _extradata = extradata;
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

    Extradata Parameters::extradata() const {
        return _extradata;
    }

    std::string Parameters::codecType() const {
        return isDecoder() ? "decoder" : "encoder";
    }

    void Parameters::increaseDuration(const int64_t value) {
        _duration += value;
    }

    std::string Parameters::toString() const {
        return utils::to_string(type()) + " "
            + codecName() + " "
            + (::av_codec_is_decoder(codec()) ? "decoder" : "encoder") + ", "
            + (bitrate() ? std::to_string(bitrate()) : "N/A") + " bit/s, "
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

    void Parameters::initCodecpar(AVCodecParameters* codecpar) const {
        codecpar->codec_type = codec()->type;
        codecpar->codec_id   = codecId();
        codecpar->codec_tag  = _codec_tag;
        codecpar->bits_per_coded_sample = int(_bits_per_coded_sample);
        codecpar->bits_per_raw_sample   = int(_bits_per_raw_sample);

        codecpar->bit_rate = bitrate();
        codecpar->profile  = _profile;
        codecpar->level    = _level;

        if (!_extradata.empty()) {
            ::av_freep(&codecpar->extradata);
            const auto extradata_size {
                size_t (_extradata.size())
            };
            codecpar->extradata
                = reinterpret_cast<uint8_t*>(::av_mallocz(extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
            if (!codecpar->extradata) {
                throw std::bad_alloc {};
            }
            ::memcpy(codecpar->extradata, _extradata.data(), extradata_size);
            codecpar->extradata_size = int(extradata_size);
        }
    }

    void Parameters::initCodecContext(AVCodecContext* codec_context) const {
        codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        codec_context->codec_type = codec()->type;
        codec_context->codec_id   = codecId();
        codec_context->codec_tag  = _codec_tag;
        codec_context->time_base  = timeBase();
        codec_context->bits_per_coded_sample = int(_bits_per_coded_sample);
        codec_context->bits_per_raw_sample   = int(_bits_per_raw_sample);

        codec_context->bit_rate = bitrate();
        codec_context->profile  = _profile;
        codec_context->level    = _level;

        if (!_extradata.empty()) {
            ::av_freep(&codec_context->extradata);
            const auto extradata_size {
                size_t (_extradata.size())
            };
            codec_context->extradata
                = reinterpret_cast<uint8_t*>(::av_mallocz(extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
            if (!codec_context->extradata) {
                throw std::bad_alloc {};
            }
            ::memcpy(codec_context->extradata, _extradata.data(), extradata_size);
            codec_context->extradata_size = int(extradata_size);
        }
    }

    void Parameters::parseCodecContext(const AVCodecContext* codec_context) {
        codec()->type = codec_context->codec_type;
        _codec_id   = codec_context->codec_id;
        _codec_tag  = codec_context->codec_tag;

        setBitrate(codec_context->bit_rate);
        _bits_per_coded_sample = codec_context->bits_per_coded_sample;
        _bits_per_raw_sample   = codec_context->bits_per_raw_sample;
        _profile               = codec_context->profile;
        _level                 = codec_context->level;

        if (codec_context->extradata) {
            setExtradata({
                codec_context->extradata
                , codec_context->extradata + codec_context->extradata_size
            });
       }
    }

    bool Parameters::betterThen(const SharedParameters& other) {
        return this->bitrate() > other->bitrate();
    }

} // namespace fpp
