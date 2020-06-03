#include "FormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <iomanip>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavdevice/avdevice.h>
}

namespace fpp {

    constexpr auto default_opening_timeout_ms { 20'000 };
    constexpr auto default_closing_timeout_ms { 5'000  };
    constexpr auto default_reading_timeout_ms { 5'000  };
    constexpr auto default_writing_timeout_ms { 1'000  };

    FormatContext::FormatContext()
        : _opened { false }
        , _timeouts {
              default_opening_timeout_ms
            , default_closing_timeout_ms
            , default_reading_timeout_ms
            , default_writing_timeout_ms
        } {
    }

    void FormatContext::close() {
        if (closed()) {
            return;
        }
        setInterruptTimeout(getTimeout(TimeoutProcess::Closing));
        closeContext();
        reset();
        setStreams({});
        setOpened(false);
    }

    bool FormatContext::opened() const {
        return _opened;
    }

    bool FormatContext::closed() const {
        return !_opened;
    }

    void FormatContext::flushContextAfterEachPacket(bool value) {
        raw()->flush_packets = int(value);
    }

    std::string FormatContext::toString() const {
        std::stringstream ss;
        ss << formatName() << ',' << ' ';
        ss << std::quoted(mediaResourceLocator()) << ':';
        for (const auto& stream : streams()) {
            ss << '\n' << stream->toString();
        }
        return ss.str();
    }

    void FormatContext::setOpened(bool opened) {
        _opened = opened;
    }

    bool FormatContext::open(Options options) {
        if (opened()) {
            log_error() << "Context already opened";
            return false;
        }
        if (!openContext(options)) {
            log_error() << "Could not open " << utils::quoted(mediaResourceLocator());
            return false;
        }
        setOpened(true);
        log_info() << toString();
        return true;
    }

    bool FormatContext::open(const std::string_view mrl, Options options) {
        setMediaResourceLocator(mrl);
        return open(options);
    }

    void FormatContext::setInterruptCallback(AVFormatContext* ctx) {
        ctx->interrupt_callback.callback = &FormatContext::interrupt_callback;
        ctx->interrupt_callback.opaque   = &_interrupter;
    }

    void FormatContext::setInterruptTimeout(int64_t timeout_ms) {
        _interrupter.set(timeout_ms);
    }

    void FormatContext::createContext() {
        reset(
            ::avformat_alloc_context()
            , [](auto* ctx) { ::avformat_free_context(ctx); }
        );
    }

    int FormatContext::interrupt_callback(void* opaque) {
        constexpr auto OK   { 0 };
        constexpr auto FAIL { 1 };
        const auto interrupter {
            reinterpret_cast<const Interrupter*>(opaque)
        };
        if (interrupter->isTimeout()) {
            static_log_error()
                << "interrupt_callback: "
                << "Timed out: " << interrupter->timeout_ms;
            return FAIL;
        }
        return OK;
    }

    const std::string_view FormatContext::mediaResourceLocator() const {
        return _media_resource_locator;
    }

    void FormatContext::setMediaResourceLocator(const std::string_view mrl) {
        _media_resource_locator = mrl;
        if (!_media_resource_locator.empty()) {
            createContext();
        }
    }

    void FormatContext::setStreams(StreamVector stream_vector) {
        _streams = stream_vector;
    }

    void FormatContext::processPacket(Packet& packet) {
        const auto packet_stream {
            stream(packet.streamIndex())
        };
        packet_stream->stampPacket(packet);
        const auto packet_type { // TODO: refactor it (03.06)
            packet_stream->timeIsOver()
                ? MediaType::EndOF
                : packet_stream->type()
        };
        packet.setType(packet_type);
    }

    void FormatContext::addStream(SharedStream stream) {
        _streams.push_back(stream);
    }

    int64_t FormatContext::streamNumber() const {
        return int64_t(raw()->nb_streams);
    }

    StreamVector FormatContext::streams() {
        return _streams;
    }

    void FormatContext::setTimeout(TimeoutProcess process, int64_t ms) {
        _timeouts[std::size_t(process)] = ms;
    }

    int64_t FormatContext::getTimeout(TimeoutProcess process) const {
        return _timeouts[std::size_t(process)];
    }

    const StreamVector FormatContext::streams() const {
        return _streams;
    }

    SharedStream FormatContext::stream(int64_t index) {
        return _streams.at(size_t(index));
    }

    SharedStream FormatContext::stream(MediaType stream_type) {
        for (const auto& stream : _streams) {
            if (stream->params->typeIs(stream_type)) {
                return stream;
            }
        }
        return nullptr;
    }

} // namespace fpp
