#include "CodecContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    CodecContext::CodecContext(const SpParameters params)
        : Media(params->type())
        , params { params } {
    }

    void CodecContext::init(Options options) {
        reset(
            ::avcodec_alloc_context3(codec())
            , [](auto* codec_ctx) {
                 ::avcodec_free_context(&codec_ctx);
            }
        );
        open(options);
    }

    void CodecContext::open(Options options) {
        params->initCodecContext(raw());
        Dictionary dictionary { options };
        if (const auto ret {
                ::avcodec_open2(raw(), codec(), dictionary.get())
            }; ret != 0) {
            throw FFmpegException {
                "Cannot open "
                    + utils::to_string(raw()->codec_type) + " "
                    + params->codecType() + " "
                    + codec()->name
            };
        }
        params->parseCodecContext(raw());
    }

    std::string CodecContext::toString() const {
        const auto separator { ", " };
        if (isVideo()) {
            return utils::to_string(raw()->codec_type) + " "
                + params->codecType() + " " + codec()->name                 + separator
                + std::to_string(raw()->bit_rate) + " bit/s"                + separator
                + "width "         + std::to_string(raw()->width)           + separator
                + "height "        + std::to_string(raw()->height)          + separator
                + "coded_width "   + std::to_string(raw()->coded_width)     + separator
                + "coded_height "  + std::to_string(raw()->coded_height)    + separator
                + "time_base "     + utils::to_string(raw()->time_base)     + separator
                + "gop "           + std::to_string(raw()->gop_size)        + separator
                + "pix_fmt "       + utils::to_string(raw()->pix_fmt);
        }
        if (isAudio()) {
            return utils::to_string(raw()->codec_type) + " "
                + params->codecType() + " " + codec()->name                 + separator
                + std::to_string(raw()->bit_rate) + " bit/s"                + separator
                + "sample_rate "   + std::to_string(raw()->sample_rate)     + separator
                + "sample_fmt "    + utils::to_string(raw()->sample_fmt)    + separator
                + "ch_layout "     + utils::channel_layout_to_string(
                                        raw()->channels
                                        , raw()->channel_layout)            + separator
                + "channels "      + std::to_string(raw()->channels)        + separator
                + "frame_size "    + std::to_string(raw()->frame_size);
        }
        throw std::runtime_error {
            "Invalid codec type"
        };
    }

    const AVCodec* CodecContext::codec() const {
        return params->codec();
    }

    bool CodecContext::opened() {
        return bool(::avcodec_is_open(raw()));
    }

} // namespace fpp
