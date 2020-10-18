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
        try {
            close();
        }
        catch(const fpp::FFmpegException& e) {
            log_error() << "FFmpegException: " << e.what();
        }
        catch (const std::exception& e) {
            log_error() << "Exception: " << e.what();
        }
        catch (...) {
            log_error() << "Unknown exception";
        }
    }

    bool InputFormatContext::seek(int stream_index, std::int64_t timestamp, SeekPrecision seek_precision) {
        const auto flags {
            [&]() -> int {
                switch (seek_precision) {
                    case SeekPrecision::Forward:
                        return 0;
                    case SeekPrecision::Backward:
                        return AVSEEK_FLAG_BACKWARD;
                    case SeekPrecision::Any:
                        return AVSEEK_FLAG_ANY;
                }
            }()
        };
        ffmpeg_api(av_seek_frame, raw(), stream_index, timestamp, flags);
        log_info() << "Success seek to " << utils::time_to_string(timestamp, DEFAULT_TIME_BASE);
        return true;
    }

    Packet InputFormatContext::read() {
        setInterruptTimeout(getTimeout(TimeoutProcess::Reading));
        auto packet { readFromSource() };
        if (!processPacket(packet)) {
            return Packet { MediaType::EndOF };
        }
        return packet;
    }

    bool InputFormatContext::openContext(const Options& options) {
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
        return std::string { raw()->iformat->name };
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

    Packet InputFormatContext::readFromSource() {
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
        return packet;
    }

} // namespace fpp
