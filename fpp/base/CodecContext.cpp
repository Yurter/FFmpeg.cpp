#include "CodecContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    CodecContext::CodecContext(const SharedParameters params)
        : MediaData(params->type())
        , params { params }
        , _opened { false } {
        setName("CodecContext");
    }

    CodecContext::~CodecContext() {
        close();
    }

    void CodecContext::init(Options options) {
        log_debug("Initialization");
        reset({
            ::avcodec_alloc_context3(codec())
            , [](auto* codec_ctx) {
                 ::avcodec_free_context(&codec_ctx);
            }
        });
        setName(name() + " " + codec()->name);
        open(options);
    }

    void CodecContext::open(Options options) {
        if (opened()) {
            throw std::runtime_error { "Codec already opened" };
        }
        log_debug("Opening");
//        if (params->isEncoder()) {
//            params->setExtradata({});
//        }
        params->initCodecContext(raw());
//        if (params->isEncoder()) {
//            if (params->isVideo()) {
//                raw()->time_base = ::av_inv_q(std::static_pointer_cast<VideoParameters>(params)->frameRate());
//            }
//        }
        Dictionary dictionary { options };
        if (const auto ret {
                ::avcodec_open2(raw(), codec(), dictionary.get())
            }; ret != 0) {
            throw FFmpegException {
                "Cannot open "
                    + utils::to_string(raw()->codec_type) + " "
                    + params->codecType() + " "
                    + codec()->name
                , ret
            };
        }
        params->parseCodecContext(raw());
//        if (params->isEncoder()) {
//            params->parseCodecContext(raw()); // TODO check why original fps 90'000 27.03
//        }
//        if (params->isAudio()) { // TODO 16.03
//            std::static_pointer_cast<AudioParameters>(params)->setFrameSize(raw()->frame_size);
//        }
//        if (params->isVideo()) { // TODO 19.03
//            if (params->isDecoder()) {
//                std::static_pointer_cast<VideoParameters>(params)->setGopSize(raw()->gop_size);
//            }
//        }
        setOpened(true);
    }

    void CodecContext::close() {
        if (closed()) {
            return;
        }
        log_debug("Closing");
        if (inited_ptr(raw())) {
            ::avcodec_close(raw());
        }
        setOpened(false);
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

    bool CodecContext::opened() const {
        return _opened;
    }

    bool CodecContext::closed() const {
        return !_opened;
    }

    void CodecContext::setOpened(bool value) {
        _opened = value;
    }

} // namespace fpp
