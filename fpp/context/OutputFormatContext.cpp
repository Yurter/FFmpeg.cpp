#include "OutputFormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    OutputFormatContext::OutputFormatContext(const std::string_view mrl)
        : FormatContext(mrl)
        , _output_format { nullptr } {
        setName("OutFmtCtx");
        createContext();
    }

    OutputFormatContext::~OutputFormatContext() {
        close();
    }

    void OutputFormatContext::write(Packet packet, WriteMode write_mode) {
        processPacket(packet);
        if (write_mode == WriteMode::Instant) {
            if (const auto ret {
                    ::av_write_frame(raw(), packet.ptr())
                }; ret < 0) {
                throw FFmpegException { "av_write_frame failed", ret };
            }
        }
        else if (write_mode == WriteMode::Interleaved) {
            if (const auto ret {
                    ::av_interleaved_write_frame(raw(), packet.ptr())
                }; ret < 0) {
                throw FFmpegException { "av_interleaved_write_frame failed", ret };
            }
        }
    }

    void OutputFormatContext::flush() {
        if (const auto ret { ::av_write_frame(raw(), nullptr) }; ret != 1) {
            throw FFmpegException { "OutputFormatContext flush failed", ret };
        }
    }

    std::string OutputFormatContext::sdp() {
        char buf[256] {};
        AVFormatContext* ctxs[] { raw() };
        if (const auto ret {
                ::av_sdp_create(ctxs, 1, buf, sizeof(buf))
            }; ret < 0) {
            throw FFmpegException {
                "av_sdp_create failed", ret
            };
        }

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
        const auto format_short_name {
            utils::guess_format_short_name(mediaResourceLocator())
        };
        AVFormatContext* fmt_ctx { nullptr };
        if (const auto ret {
                ::avformat_alloc_output_context2(
                    &fmt_ctx                            /* ctx          */
                    , nullptr                           /* oformat      */
                    , format_short_name                 /* format_name  */
                    , mediaResourceLocator().c_str()    /* filename     */
                )
            }; ret < 0) {
            throw FFmpegException { "avformat_alloc_output_context2 failed", ret };
        }
        reset(std::shared_ptr<AVFormatContext> {
                fmt_ctx
                , [](auto* ctx) { ::avformat_free_context(ctx); }
            }
        );
    }

    void OutputFormatContext::openContext() {
        if (streamNumber() == 0) {
            throw std::logic_error { "Can't open context without streams" };
        }
        if (!(raw()->flags & AVFMT_NOFILE)) {
            if (const auto ret {
                    ::avio_open(
                        &raw()->pb                       /* AVIOContext */
                        , mediaResourceLocator().c_str() /* url         */
                        , AVIO_FLAG_WRITE                /* flags       */
                    )
                }; ret < 0) {
                throw FFmpegException {
                    "Could not open output: " + mediaResourceLocator()
                    , ret
                };
            }
        }
        writeHeader();
        parseStreamsTimeBase();
    }

    std::string OutputFormatContext::formatName() const {
        return raw()->oformat->name;
    }

    void OutputFormatContext::beforeCloseContext() {
        writeTrailer();
    }

    SharedStream OutputFormatContext::createStream(SharedParameters params) {
        const auto avstream  { ::avformat_new_stream(raw(), params->codec()) };
        const auto fppstream { Stream::make_output_stream(avstream, params)  };
        addStream(fppstream);
        return fppstream;
    }

    SharedStream OutputFormatContext::copyStream(const SharedStream other, SharedParameters output_params) {
        const auto input_params { other->params };
        const auto full_stream_copy { !output_params };
        if (full_stream_copy) {
            output_params = utils::make_params(input_params->type());
        }
        output_params->completeFrom(input_params);
        const auto created_stream { createStream(output_params) };
        if (full_stream_copy) {
            if (const auto ret {
                ::avcodec_parameters_copy(
                    created_stream->codecpar() /* dst */
                    , other->codecpar()        /* src */
                )
            }; ret < 0) {
                throw FFmpegException {
                    "Could not copy stream codec parameters!"
                    , ret
                };
            }
        }
        return created_stream;
    }

    Code OutputFormatContext::guessOutputFromat() {
        const auto out_fmt {
            ::av_guess_format(
                nullptr                             /* short_name */
                , mediaResourceLocator().c_str()    /* filename   */
                , nullptr                           /* mime_type  */
            )
        };
        setOutputFormat(out_fmt);
        return Code::OK;
    }

    StreamVector OutputFormatContext::parseFormatContext() {
        throw std::logic_error { "OutputFormatContext::parseFormatContext()" };
    }

    void OutputFormatContext::writeHeader() {
        ::avformat_write_header(
            raw()
            , nullptr /* options */
        );
    }

    void OutputFormatContext::writeTrailer() {
        if (const auto ret { ::av_write_trailer(raw()) }; ret < 0) {
            throw FFmpegException {
                "Failed to write stream trailer to " + mediaResourceLocator()
                , ret
            };
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
