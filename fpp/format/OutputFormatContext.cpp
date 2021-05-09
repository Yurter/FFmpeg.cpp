#include "OutputFormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

OutputFormatContext::OutputFormatContext(const std::string_view mrl, const std::string_view format)
    : _output_format { findOutputFormat(format) } {
    setMediaResourceLocator(mrl);
    createContext();
}

OutputFormatContext::OutputFormatContext(OutputContext* output_ctx, const std::string_view format)
    : _output_format { findOutputFormat(format) } {
    setMediaResourceLocator("Custom output buffer");
    createContext();
    raw()->pb = output_ctx->raw();
    setFlag(AVFMT_FLAG_CUSTOM_IO);
    setFlag(AVFMT_NOFILE);
}

OutputFormatContext::~OutputFormatContext() {
    try {
        close();
    }
    catch (...) {
        utils::handle_exceptions(this);
    }
}

bool OutputFormatContext::write(Packet packet) {
    if (!processPacket(packet)) {
        return false;
    }
    setInterruptTimeout(getTimeout(TimeoutProcess::Writing));
    ffmpeg_api_non_strict(av_write_frame, raw(), packet.ptr());
    return true;
}

bool OutputFormatContext::interleavedWrite(Packet& packet) {
    processPacket(packet);
    if (packet.isEOF()) {
        return false;
    }
    setInterruptTimeout(getTimeout(TimeoutProcess::Writing));
    ffmpeg_api_non_strict(av_interleaved_write_frame, raw(), packet.ptr());
    return true;
}

void OutputFormatContext::flush() {
    ffmpeg_api_strict(av_write_frame, raw(), nullptr);
}

std::string OutputFormatContext::sdp() { // TODO: move method to utils & make arg array of OFC (05.06)
    char buf[256] {};
    AVFormatContext* ctxs[] { raw() }; // TODO do not use utils::merge_sdp_files(), instead use ctxs array 09.04
    ffmpeg_api_strict(av_sdp_create
        , ctxs
        , 1
        , buf
        , sizeof(buf)
    );

    std::string sdp_str { buf };
    sdp_str.append("\n");

    const auto remove_bug_param {
        [](std::string& str) {
            if (const auto pos_begin_param {
                str.find("; sprop-parameter-sets")
            }; pos_begin_param != std::string::npos) {
                str.erase(
                    pos_begin_param
                    , str.find('\n', pos_begin_param) - pos_begin_param
                );
            }
            return str;
        }
    };

    remove_bug_param(sdp_str);

    return sdp_str;
}

void OutputFormatContext::createContext() {
    reset(
        [this]() {
            const auto format_short_name {
                utils::guess_format_short_name(mediaResourceLocator())
            };
            AVFormatContext* fmt_ctx { nullptr };
            ffmpeg_api_strict(avformat_alloc_output_context2
                , &fmt_ctx
                , outputFormat()
                , format_short_name.data()
                , mediaResourceLocator().data()
            );
            setOutputFormat(fmt_ctx->oformat);
            return fmt_ctx;
        }()
        , [](auto* ctx) { ::avformat_free_context(ctx); }
    );
}

bool OutputFormatContext::openContext(const Options& options) {
    if (streamNumber() == 0) {
        log_error() << "Can't open context without streams";
        return false;
    }
    initStreamsCodecpar();
    Dictionary dictionary { options };
    if (!isFlagSet(AVFMT_NOFILE)) {
        if (const auto ret {
                ::avio_open2(
                      &raw()->pb                    /* AVIOContext        */
                    , mediaResourceLocator().data() /* url                */
                    , AVIO_FLAG_WRITE               /* flags              */
                    , nullptr                       /* interrupt callback */
                    , dictionary.get()              /* options            */
                )
            }; ret < 0) {
            return false;
        }
    }
    writeHeader(options);
    parseStreamsTimeBase();
    setOutputFormat(raw()->oformat);
    return true;
}

std::string OutputFormatContext::formatName() const {
    return std::string { raw()->oformat->name };
}

void OutputFormatContext::closeContext() {
    writeTrailer();
    if (!isFlagSet(AVFMT_NOFILE)) {
        ffmpeg_api_strict(avio_close, raw()->pb);
    }
    setOutputFormat(nullptr);
}

void OutputFormatContext::createStream(SpParameters params) {
    params->setFormatFlags(outputFormat()->flags);
    const auto avstream  { ::avformat_new_stream(raw(), params->codec()) };
    const auto fppstream { Stream::make_output_stream(avstream, params)  };
    addStream(fppstream);
}

void OutputFormatContext::copyStream(const SharedStream other) {
    const auto output_params { utils::make_params(other->params->type()) };
    output_params->completeFrom(other->params);
    createStream(output_params);
}

void OutputFormatContext::guessOutputFromat() {
    const auto out_fmt {
        ::av_guess_format(
              nullptr                       /* short name */
            , mediaResourceLocator().data() /* filename   */
            , nullptr                       /* mime type  */
        )
    };
    if (!out_fmt) {
        throw FFmpegException {
            "av_guess_format failed"
        };
    }
    setOutputFormat(out_fmt);
}

AVOutputFormat* OutputFormatContext::findOutputFormat(const std::string_view short_name) const {
    return ::av_guess_format(short_name.data(), nullptr, nullptr);
}

void OutputFormatContext::writeHeader(const Options& options) {
    Dictionary dictionary { options };
    ::avformat_write_header(
          raw()            /* AVFormatContext */
        , dictionary.get() /* options         */
    );
}

void OutputFormatContext::writeTrailer() {
    ffmpeg_api_strict(av_write_trailer, raw());
}

void OutputFormatContext::initStreamsCodecpar() {
    for (const auto& stream : streams()) {
        stream->initCodecpar();
    }
}

void OutputFormatContext::parseStreamsTimeBase() {
    for (const auto& stream : streams()) {
        stream->params->setTimeBase(stream->raw()->time_base);
    }
}

AVOutputFormat* OutputFormatContext::outputFormat() {
    return _output_format;
}

void OutputFormatContext::setOutputFormat(AVOutputFormat* out_fmt) {
    _output_format = out_fmt;
}

} // namespace fpp
