#include "Parameters.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

constexpr auto not_inited_codec_id { [](auto x) { return x == AVCodecID::AV_CODEC_ID_NONE; } };

namespace fpp {

    Parameters::Parameters(MediaType type)
        : MediaData(type)
        , _codec { nullptr }
        , _time_base { DEFAULT_RATIONAL }
        , _format_flags { 0 } {
        setName("Parameters");
        reset();
    }

    Parameters::Parameters(const Parameters& other) :
        MediaData(other.type()) {
        ffmpeg_api_strict(avcodec_parameters_copy, ptr(), other.ptr());
        _codec = other.codec();
        _time_base = other.timeBase();
        _format_flags = other.formatFlags();
    }

    Parameters& Parameters::operator=(const Parameters& other) {
        setType(other.type());
        ffmpeg_api_strict(avcodec_parameters_copy, ptr(), other.ptr());
        _codec = other.codec();
        _time_base = other.timeBase();
        _format_flags = other.formatFlags();
        return *this;
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
        if (_codec) {
            raw().codec_id = _codec->id;
        }
    }

    bool Parameters::testFormatFlag(int flag) const {
        return _format_flags & flag;
    }

    void Parameters::setBitrate(int64_t bitrate) {
        raw().bit_rate = bitrate;
    }

    void Parameters::setTimeBase(AVRational time_base) {
        _time_base = time_base;
    }

    void Parameters::setExtradata(Extradata extradata) {
        ::av_freep(&raw().extradata);
        const auto& [data,data_size] { extradata };
        if (data_size != 0) {
            raw().extradata = reinterpret_cast<uint8_t*>(
                ::av_mallocz(data_size + AV_INPUT_BUFFER_PADDING_SIZE)
            );
            if (!raw().extradata) {
                throw std::bad_alloc {};
            }
            ::memcpy(raw().extradata, data, data_size);
        }
        raw().extradata_size = int(data_size);
    }

    void Parameters::setFormatFlags(int flags) {
        _format_flags = flags;
    }

    AVCodecID Parameters::codecId() const {
        return raw().codec_id;
    }

    std::string Parameters::codecName() const {
        if (!_codec) {
            return "none";
        }
        return _codec->name;
    }

    AVCodec* Parameters::codec() const {
        return _codec;
    }

    int64_t Parameters::bitrate() const {
        return raw().bit_rate;
    }

    AVRational Parameters::timeBase() const {
        return _time_base;
    }

    Extradata Parameters::extradata() const {
        return { raw().extradata, raw().extradata_size };
    }

    std::string Parameters::codecType() const {
        return isDecoder() ? "decoder" : "encoder";
    }

    int Parameters::formatFlags() const {
        return _format_flags;
    }

    std::string Parameters::toString() const {
        return utils::to_string(type()) + " "
            + codecName()
            + (codec()
                ? std::string { " " }
                    + (::av_codec_is_decoder(codec()) ? "decoder" : "encoder") + ", "
                    + (bitrate() ? std::to_string(bitrate()) : "N/A") + " bit/s, "
                    + "tb " + utils::to_string(timeBase())
               : "");
    }

    void Parameters::completeFrom(const SpParameters other) {
        if (extradata().second == 0)        { setExtradata(other->extradata()); }
        if (not_inited_codec_id(codecId())) { setEncoder(other->codecId());     }
        if (not_inited_int(bitrate()))      { setBitrate(other->bitrate());     }
        if (not_inited_q(timeBase()))       { setTimeBase(other->timeBase());   }
    }

    void Parameters::parseStream(const AVStream* avstream) {
        parseCodecpar(avstream->codecpar);
        setDecoder(codecId());
        setTimeBase(avstream->time_base);
    }

    void Parameters::initCodecpar(AVCodecParameters* codecpar) const {
        ffmpeg_api_strict(avcodec_parameters_copy, codecpar, ptr());
    }

    void Parameters::parseCodecpar(AVCodecParameters* codecpar) {
        ffmpeg_api_strict(avcodec_parameters_copy, ptr(), codecpar);
    }

    void Parameters::initCodecContext(AVCodecContext* codec_context) const {
        ffmpeg_api_strict(avcodec_parameters_to_context, codec_context, ptr());

        if (testFormatFlag(AVFMT_GLOBALHEADER)) {
            codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        if (isEncoder()) { // TODO: check it, do not write extradata of input stream to encoder if stream transcoded (27.04)
            if (codec_context->extradata) {
                ::av_freep(&codec_context->extradata);
                codec_context->extradata_size = 0;
            }
        }

//        codec_context->time_base = timeBase();
//        codec_context->time_base = AVRational { 1, framerate };

    }

    void Parameters::parseCodecContext(const AVCodecContext* codec_context) {
        ffmpeg_api_strict(avcodec_parameters_from_context, ptr(), codec_context);
    }

    void Parameters::reset() {
        if (raw().extradata) {
            ::av_freep(raw().extradata);
        }

        ::memset(ptr(), 0, sizeof(raw()));

//        raw().codec_type          = AVMEDIA_TYPE_UNKNOWN;
        raw().codec_type          = utils::from_media_type(type());
        raw().codec_id            = AV_CODEC_ID_NONE;
        raw().format              = -1;
        raw().field_order         = AV_FIELD_UNKNOWN;
        raw().color_range         = AVCOL_RANGE_UNSPECIFIED;
        raw().color_primaries     = AVCOL_PRI_UNSPECIFIED;
        raw().color_trc           = AVCOL_TRC_UNSPECIFIED;
        raw().color_space         = AVCOL_SPC_UNSPECIFIED;
        raw().chroma_location     = AVCHROMA_LOC_UNSPECIFIED;
        raw().sample_aspect_ratio = AVRational { 0, 1 };
        raw().profile             = FF_PROFILE_UNKNOWN;
        raw().level               = FF_LEVEL_UNKNOWN;
    }

    bool Parameters::betterThen(const SpParameters& other) {
        return this->bitrate() > other->bitrate();
    }

} // namespace fpp
