#include "InputFormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    InputFormatContext::InputFormatContext(const std::string_view mrl)
        : FormatContext(mrl)
        , _input_format { nullptr } {
        setName("InpFmtCtx");
        createContext();
    }

    InputFormatContext::~InputFormatContext() {
        close();
    }

    void InputFormatContext::seek(int64_t stream_index, int64_t timestamp, SeekPrecision seek_precision) {
        auto flags { 0 };
        switch (seek_precision) {
        case SeekPrecision::Forward:
            flags = 0;
            break;
        case SeekPrecision::Backward:
            flags = AVSEEK_FLAG_BACKWARD;
            break;
        case SeekPrecision::Any:
            flags = AVSEEK_FLAG_ANY;
            break;
        case SeekPrecision::Precisely:
            throw std::runtime_error { "NOT_IMPLEMENTED" };
        }
        if (const auto ret {
                ::av_seek_frame(raw(), int(stream_index), timestamp, flags)
            }; ret < 0) {
            throw FFmpegException {
                "Failed to seek timestamp "
                    + utils::time_to_string(timestamp, DEFAULT_TIME_BASE)
                    + " in stream " + std::to_string(stream_index)
                , ret
            };
        }
        log_info("Success seek to " << utils::time_to_string(timestamp, DEFAULT_TIME_BASE));
    }

    Packet InputFormatContext::read() {
        Packet packet { MediaType::Unknown };
        if (const auto ret { ::av_read_frame(raw(), &packet.raw()) }; ret < 0) {
            if (ret == AVERROR_EOF) {
                return Packet { MediaType::EndOF };
            }
            throw FFmpegException {
                "Cannot read source: \'"
                    + mediaResourceLocator() + "\'"
                , ret
            };
        }
        processPacket(packet);
        return packet;
    }

    void InputFormatContext::createContext() {
        reset(std::shared_ptr<AVFormatContext> {
            ::avformat_alloc_context()
            , [](auto* ctx) { ::avformat_free_context(ctx); }
        });
    }

    void InputFormatContext::openContext() {
        guessInputFromat();
        auto fmt_ctx { raw() };
        if (const auto ret {
                ::avformat_open_input(
                    &fmt_ctx                            /* AVFormatContext  */
                    , mediaResourceLocator().c_str()    /* url              */
                    , inputFormat()                     /* AVInputFormat    */
                    , nullptr                           /* AVDictionary     */
                )
            }; ret < 0) {
            throw FFmpegException { "Failed to open input context", ret };
        }
        setInputFormat(raw()->iformat);
        if (const auto ret { ::avformat_find_stream_info(raw(), nullptr) }; ret < 0 ) {
            throw FFmpegException { "Failed to retrieve input stream information", ret };
        }
        setStreams(parseFormatContext());
    }

    StreamVector InputFormatContext::parseFormatContext() {
        StreamVector result;
        for (unsigned i { 0 }; i < raw()->nb_streams; ++i) {
            result.push_back(make_input_stream(raw()->streams[i]));
        }
        return result;
    }

    void InputFormatContext::guessInputFromat() {
        const auto short_name { utils::guess_format_short_name(mediaResourceLocator()) };
        setInputFormat(::av_find_input_format(short_name));
    }

    AVInputFormat* InputFormatContext::inputFormat() {
        return _input_format;
    }

    void InputFormatContext::setInputFormat(AVInputFormat* in_fmt) {
        _input_format = in_fmt;
    }

} // namespace fpp
