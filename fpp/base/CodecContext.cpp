#include "CodecContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    CodecContext::CodecContext(const SharedParameters parameters, const AVStream* test_stream)
        : params { parameters }
        , _opened { false }
        , _test_stream { test_stream } {
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
        initContextParams();
        if (true) { // TODO костыль
            raw()->time_base = params->timeBase();
        }
        open(std::move(dictionary));
    }

    void CodecContext::open(Dictionary&& dictionary) {
        if (opened()) {
            throw std::runtime_error { "Codec already opened" };
        }
        int ret = avcodec_parameters_from_context(_test_stream->codecpar, raw());
        if (ret < 0)
        {
            fprintf(stderr, "Could not initialize stream codec parameters!\n");
            return;
        }
        log_debug("Opening");
        if (const auto ret { ::avcodec_open2(raw(), codec(), dictionary.ptrPtr()) }; ret != 0) {
            const auto codec_type {
                ::av_codec_is_decoder(codec()) ? "decoder" : "encoder"
            };
            throw FFmpegException {
                "Cannot open " + std::string { codec()->name } + ", " + codec_type
                , ret
            };
        }
        onOpen();
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

    void CodecContext::initContextParams() { //TODO
        raw()->codec_id = params->codecId();
        raw()->bit_rate = params->bitrate();
//        codec->time_base = param->timeBase(); // TODO см :318 12/02
//        codec_context->time_base = { 1, 16000/*тут нужен таймбейс входного потока, либо рескейлить в энкодере*/ };

        if (params->isVideo()) {
            const auto video_parameters { std::static_pointer_cast<const VideoParameters>(params) };
            raw()->pix_fmt      = video_parameters->pixelFormat();
            raw()->width        = int(video_parameters->width());
            raw()->height       = int(video_parameters->height());
//            codec->time_base    = param->timeBase(); // TODO почему закомментировано ? см CodecContex.cpp:28 12.02
            raw()->framerate    = video_parameters->frameRate();
            raw()->gop_size     = int(video_parameters->gopSize());
    //        codec->sample_aspect_ratio    = video_parameters->sampl; //TODO

            raw()->codec_tag = 0;
            raw()->codec_type = AVMEDIA_TYPE_VIDEO;
            raw()->gop_size = 12;
            raw()->framerate = { 30, 0 };
            raw()->time_base = { 1, 1000 };
//            raw()->bit_rate = 500000;
            raw()->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            return;
        }

        if (params->isAudio()) {
            const auto audio_parameters { std::static_pointer_cast<const AudioParameters>(params) };
            raw()->sample_fmt       = audio_parameters->sampleFormat();
            raw()->channel_layout   = audio_parameters->channelLayout();
            raw()->channels         = int(audio_parameters->channels());
            raw()->sample_rate      = int(audio_parameters->sampleRate());
//            raw()->profile = FF_PROFILE_MPEG2_AAC_LOW;
//            raw()->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//            raw()->codec_tag = 0;
            raw()->bit_rate = 50000;
            raw()->codec_type = AVMEDIA_TYPE_AUDIO;
            return;
        }

        throw std::invalid_argument {
            "Failed to init codec's params with "
                + utils::to_string(params->type()) + " params"
        };
    }

} // namespace fpp
