#include "OutputFormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    OutputFormatContext::OutputFormatContext(const std::string_view mrl)
        : FormatContext { mrl }
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
            if (const auto ret { ::av_write_frame(raw(), packet.ptr()) }; ret < 0) {
                throw FFmpegException { "av_write_frame failed", ret };
            }
        }
        else if (write_mode == WriteMode::Interleaved) {
            if (const auto ret { ::av_interleaved_write_frame(raw(), packet.ptr()) }; ret < 0) {
                throw FFmpegException { "av_interleaved_write_frame failed", ret };
            }
        }
    }

    void OutputFormatContext::flush() {
        if (const auto ret { ::av_write_frame(raw(), nullptr) }; ret != 1) {
            throw FFmpegException { "av_write_frame failed", ret };
        }
    }

    void OutputFormatContext::createContext() {
        const auto format_short_name { utils::guess_format_short_name(mediaResourceLocator()) };
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
                , [](AVFormatContext* ctx) { ::avformat_free_context(ctx); }
            }
        );
    }

    void OutputFormatContext::openContext() {
        if (!(raw()->flags & AVFMT_NOFILE)) {
            if (const auto ret {
                    ::avio_open(
                        &raw()->pb                          /* AVIOContext  */
                        , mediaResourceLocator().c_str()    /* url          */
                        , AVIO_FLAG_WRITE                   /* flags        */
                    )
                }; ret < 0) {
                throw FFmpegException {
                    "Could not open output: " + mediaResourceLocator()
                    , ret
                };
            }
        }
        if (::avformat_write_header(raw(), nullptr) < 0) {
            throw FFmpegException { "avformat_write_header failed" };
        }
    }

    void OutputFormatContext::closeContext() {
        if (const auto ret { ::av_write_trailer(raw()) }; ret < 0) {
            throw FFmpegException {
                "Failed to write stream trailer to " + mediaResourceLocator()
                , ret
            };
        }
        if (const auto ret { ::avio_close(raw()->pb) }; ret < 0) {
            throw FFmpegException {
                "Failed to close " + mediaResourceLocator()
                , ret
            };
        }
    }

    SharedStream OutputFormatContext::createStream(SharedParameters params) {
        const auto avstream { ::avformat_new_stream(raw(), params->codec()) };
        return std::make_shared<Stream>(avstream, params);
    }

    Code OutputFormatContext::guessOutputFromat() {
        auto out_fmt { ::av_guess_format(nullptr, mediaResourceLocator().c_str(), nullptr) };
        setOutputFormat(out_fmt);
        return Code::OK;
    }

    StreamVector OutputFormatContext::parseFormatContext() {
        throw std::logic_error { "OutputFormatContext::parseFormatContext()" };
    }

    AVOutputFormat* OutputFormatContext::outputFormat() {
        return _output_format;
    }

    void OutputFormatContext::setOutputFormat(AVOutputFormat* out_fmt) {
        _output_format = out_fmt;
    }

} // namespace fpp
