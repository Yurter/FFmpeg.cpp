#include "InputFormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    InputFormatContext::InputFormatContext(const std::string_view mrl, const std::string_view format_short_name)
        : FormatContext(mrl)
        , _input_format { findInputFormat(format_short_name.data()) } {
        setName("InFmtCtx");
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
        setInterrupter(timeoutReading());
        Packet packet { MediaType::Unknown };
        if (const auto ret { ::av_read_frame(raw(), packet.ptr()) }; ret < 0) {
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

    bool InputFormatContext::openContext(Options options) {
        if (!inputFormat()) {
            guessInputFromat();
        }
        Dictionary dictionary { options };
        AVFormatContext* fmt_ctx { ::avformat_alloc_context() };
        setInterruptCallback(fmt_ctx);
        setInterrupter(timeoutOpening());
        if (const auto ret {
                ::avformat_open_input(
                    &fmt_ctx
                    , mediaResourceLocator().c_str()
                    , inputFormat()
                    , dictionary.get()
                )
            }; ret < 0) {
            return false;
        }
        reset(std::shared_ptr<AVFormatContext> {
            fmt_ctx
            , [](auto* ctx) { /*::avformat_free_context(ctx);*/ } // TODO avformat_close_input free context 10.04
        });
        setInputFormat(raw()->iformat);
        if (const auto ret {
                ::avformat_find_stream_info(raw(), nullptr)
            }; ret < 0 ) {
            throw FFmpegException {
                "Failed to retrieve input stream information"
                , ret
            };
        }
        setStreams(parseFormatContext());
        return true;
    }

    std::string InputFormatContext::formatName() const {
        return raw()->iformat->name;
    }

    void InputFormatContext::closeContext() {
        auto ctx { raw() }; // TODO 10.04
        ::avformat_close_input(&ctx);
        _input_format = nullptr;
    }

    StreamVector InputFormatContext::parseFormatContext() {
        StreamVector result;
        for (auto i { 0u }; i < raw()->nb_streams; ++i) {
            result.push_back(Stream::make_input_stream(raw()->streams[i]));
            result.back()->params->setFormatFlags(inputFormat()->flags);
        }
        return result;
    }

    void InputFormatContext::guessInputFromat() {
        const auto short_name {
            utils::guess_format_short_name(mediaResourceLocator())
        };
        setInputFormat(findInputFormat(short_name));
    }

    AVInputFormat* InputFormatContext::findInputFormat(const char* short_name) const {
        if (short_name) {
            if (std::string { short_name } == "dshow") {
                utils::device_register_all();
            }
            else if (std::string { short_name } == "gdigrab") {
                utils::device_register_all();
            }
            else if (std::string { short_name } == "lavfi") {
                utils::device_register_all();
            }
        }
        return ::av_find_input_format(short_name);
    }

    AVInputFormat* InputFormatContext::inputFormat() {
        return _input_format;
    }

    void InputFormatContext::setInputFormat(AVInputFormat* in_fmt) {
        _input_format = in_fmt;
    }

} // namespace fpp
