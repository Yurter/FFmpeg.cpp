#include "OutputFormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    OutputFormatContext::OutputFormatContext(const std::string_view mrl)
        : _output_format { nullptr } {
        setMediaResourceLocator(mrl);
    }

    OutputFormatContext::~OutputFormatContext() {
        close();
    }

    bool OutputFormatContext::write(Packet packet, WriteMode write_mode) {
        processPacket(packet);
        if (packet.isEOF()) {
            return false;
        }
        setInterruptTimeout(getTimeout(TimeoutProcess::Writing));
        if (write_mode == WriteMode::Instant) {
            ffmpeg_api(av_write_frame, raw(), packet.ptr());
        }
        else if (write_mode == WriteMode::Interleaved) {
            ffmpeg_api(av_interleaved_write_frame, raw(), packet.ptr());
        }
        return true;
    }

    void OutputFormatContext::flush() {
        ffmpeg_api_strict(av_write_frame, raw(), nullptr);
    }

    std::string OutputFormatContext::sdp() {
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
        reset(
            fmt_ctx
            , [](auto* ctx) { ::avformat_free_context(ctx); }
        );
    }

    bool OutputFormatContext::openContext(Options options) {
        if (streamNumber() == 0) {
            throw std::logic_error {
                "Can't open context without streams"
            };
        }
        initStreamsCodecpar();
        Dictionary dictionary { options };
        if (!(raw()->flags & AVFMT_NOFILE)) {
            if (const auto ret {
                    ::avio_open2(
                        &raw()->pb                      /* AVIOContext */
                        , mediaResourceLocator().data() /* url         */
                        , AVIO_FLAG_WRITE               /* flags       */
                        , nullptr                       /* int_cb      */
                        , dictionary.get()
                    )
                }; ret < 0) {
                return false;
            }
        }
        writeHeader();
        parseStreamsTimeBase();
        setOutputFormat(raw()->oformat);
        return true;
    }

    std::string OutputFormatContext::formatName() const {
        return raw()->oformat->name;
    }

    void OutputFormatContext::closeContext() {
        writeTrailer();
        if (!(outputFormat()->flags & AVFMT_NOFILE)) {
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
                nullptr                         /* short_name */
                , mediaResourceLocator().data() /* filename   */
                , nullptr                       /* mime_type  */
            )
        };
        if (!out_fmt) {
            throw FFmpegException {
                "av_guess_format failed"
            };
        }
        setOutputFormat(out_fmt);
    }

    void OutputFormatContext::writeHeader() { // TODO use options 09.04
        ::avformat_write_header(
            raw()
            , nullptr /* options */
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
