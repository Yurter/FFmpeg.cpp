#include "InputFormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

    InputFormatContext::InputFormatContext(const std::string_view mrl, const std::string_view format_short_name)
        : _input_format { findInputFormat(format_short_name) } {
        setMediaResourceLocator(mrl);
    }

    InputFormatContext::~InputFormatContext() {
        close();
    }

    void InputFormatContext::seek(int64_t stream_index, int64_t timestamp, SeekPrecision seek_precision) {
        const auto flags {
            [&]() {
                switch (seek_precision) {
                    case SeekPrecision::Forward:
                        return 0;
                    case SeekPrecision::Backward:
                        return AVSEEK_FLAG_BACKWARD;
                    case SeekPrecision::Any:
                        return AVSEEK_FLAG_ANY;
                    case SeekPrecision::Precisely:
                        throw std::runtime_error { "NOT_IMPLEMENTED" };
                }

            }()
        };
        if (const auto ret {
                ::av_seek_frame(raw(), int(stream_index), timestamp, flags)
            }; ret < 0) {
            throw FFmpegException {
                "Failed to seek timestamp "
                    + utils::time_to_string(timestamp, DEFAULT_TIME_BASE)
                    + " in stream " + std::to_string(stream_index)
            };
        }
        log_info("Success seek to ", utils::time_to_string(timestamp, DEFAULT_TIME_BASE));
    }

    Packet InputFormatContext::read() {
        setInterruptTimeout(getTimeout(TimeoutProcess::Reading));
        Packet packet { MediaType::Unknown };
        if (const auto ret { ::av_read_frame(raw(), packet.ptr()) }; ret < 0) {
            if (ret == AVERROR_EOF) {
                return Packet { MediaType::EndOF };
            }
            throw FFmpegException {
                "Cannot read source: "
                    + utils::quoted(mediaResourceLocator())
            };
        }
        processPacket(packet);
        return packet;
    }

    bool InputFormatContext::openContext(Options options) {
        if (!inputFormat()) {
            guessInputFromat();
        }
        reset(
            [&]() -> AVFormatContext* {
                AVFormatContext* fmt_ctx {
                    ::avformat_alloc_context()
                };
                setInterruptCallback(fmt_ctx);
                setInterruptTimeout(getTimeout(TimeoutProcess::Opening));
                Dictionary dictionary { options };
                if (const auto ret {
                        ::avformat_open_input(
                            &fmt_ctx
                            , mediaResourceLocator().data()
                            , inputFormat()
                            , dictionary.get()
                        )
                    }; ret < 0) {
                    return nullptr;
                }
                return fmt_ctx;
            }()
            , [](auto* ctx) { ::avformat_close_input(&ctx); }
        );
        if (isNull()) {
            return false;
        }
        setInputFormat(raw()->iformat);
        retrieveStreams();
        return true;
    }

    std::string InputFormatContext::formatName() const {
        return raw()->iformat->name;
    }

    void InputFormatContext::closeContext() {
        reset();
        setInputFormat(nullptr);
    }

    void InputFormatContext::retrieveStreams() {
        if (const auto ret {
                ::avformat_find_stream_info(raw(), nullptr) // TODO: use options (12.05)
            }; ret < 0 ) {
            throw FFmpegException {
                "Failed to retrieve input stream information"
            };
        }
        StreamVector result;
        for (auto i { 0u }; i < raw()->nb_streams; ++i) {
            result.push_back(Stream::make_input_stream(raw()->streams[i]));
            result.back()->params->setFormatFlags(inputFormat()->flags);
        }
        setStreams(result);
    }

    void InputFormatContext::guessInputFromat() {
        const auto short_name {
            utils::guess_format_short_name(mediaResourceLocator())
        };
        setInputFormat(findInputFormat(short_name));
    }

    AVInputFormat* InputFormatContext::findInputFormat(const std::string_view short_name) const {
        if (!short_name.empty()) {
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
        return ::av_find_input_format(short_name.data());
    }

    AVInputFormat* InputFormatContext::inputFormat() {
        return _input_format;
    }

    void InputFormatContext::setInputFormat(AVInputFormat* in_fmt) {
        _input_format = in_fmt;
    }

} // namespace fpp
