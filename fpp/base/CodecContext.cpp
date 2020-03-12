#include "CodecContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    CodecContext::CodecContext(const SharedStream stream)
        : params { stream->params }
        , _stream { stream }
        , _opened { false } {
        setName("CodecContext");
    }

    CodecContext::~CodecContext() {
        close();
    }

    void CodecContext::init(Dictionary&& dictionary) {
        log_debug("Initialization");
        reset({
            ::avcodec_alloc_context3(codec())
            , [](auto* codec_ctx) {
                 ::avcodec_free_context(&codec_ctx);
            }
        });
        setName(name() + " " + codec()->name);
        open(std::move(dictionary));
    }

    void CodecContext::open(Dictionary&& dictionary) {
        if (opened()) {
            throw std::runtime_error { "Codec already opened" };
        }
        log_debug("Opening");
        initContext();
        if (const auto ret {
                ::avcodec_open2(raw(), codec(), dictionary.ptrPtr())
            }; ret != 0) {
            const auto codec_type {
                _stream->params->isDecoder() ? "decoder" : "encoder"
            };
            throw FFmpegException {
                "Cannot open " + std::string { codec()->name } + ", " + codec_type
                , ret
            };
        }
        if (_stream->params->isEncoder()) {
            initStreamCodecpar();
        }
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

    std::string CodecContext::toString() const { // TODO лог не подоходит для аудио 13.02
        const auto delimeter { ", " };
        return "Codec name: "   + std::string(raw()->codec->name)      + delimeter
            + "codec id: "      + utils::to_string(raw()->codec->id)   + delimeter
            + "codec type: "    + utils::to_string(raw()->codec_type)  + delimeter
            + "width: "         + std::to_string(raw()->width)         + delimeter
            + "height: "        + std::to_string(raw()->height)        + delimeter
            + "coded_width: "   + std::to_string(raw()->coded_width)   + delimeter
            + "coded_height: "  + std::to_string(raw()->coded_height)  + delimeter
            + "time_base: "     + utils::to_string(raw()->time_base)   + delimeter
            + "pix_fmt: "       + utils::to_string(raw()->pix_fmt);
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

    void CodecContext::initContext() {
        if (const auto ret {
            ::avcodec_parameters_to_context(raw(), _stream->raw()->codecpar)
        }; ret < 0) {
            throw FFmpegException {
                "Could not initialize stream codec parameters!"
                , ret
            };
        }
        raw()->time_base = params->timeBase();
    }

    void CodecContext::initStreamCodecpar() {
        if (const auto ret {
            ::avcodec_parameters_from_context(_stream->raw()->codecpar, raw())
        }; ret < 0) {
            throw FFmpegException {
                "Could not initialize stream codec parameters!"
                , ret
            };
        }
    }

} // namespace fpp
