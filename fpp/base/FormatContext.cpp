#include "FormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <iomanip>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavdevice/avdevice.h>
}

namespace fpp {

    FormatContext::FormatContext()
        : _opened { false }
        , _timeout_opening { 20'000 }
        , _timeout_closing { 5'000 }
        , _timeout_reading { 5'000 }
        , _timeout_writing { 1'000 } {
        setName("FormatContext");
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
            log_error("Context already opened");
            return false;
        }
        if (!openContext(options)) {
            log_error("Could not open ", utils::quoted(mediaResourceLocator()));
            return false;
        }
        setOpened(true);
        log_info(toString());
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
            static_log_error(
                "interrupt_callback"
                , "Timed out: ", interrupter->timeout_ms
            );
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
        const auto packet_type {
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
        switch (process) {
            case TimeoutProcess::Opening: {
                _timeout_opening = ms;
                return;
            }
            case TimeoutProcess::Closing: {
                _timeout_closing = ms;
                return;
            }
            case TimeoutProcess::Reading: {
                _timeout_reading = ms;
                return;
            }
            case TimeoutProcess::Writing: {
                _timeout_writing = ms;
                return;
            }
        }
    }

    int64_t FormatContext::getTimeout(TimeoutProcess process) const {
        switch (process) {
            case TimeoutProcess::Opening: {
                return _timeout_opening;
            }
            case TimeoutProcess::Closing: {
                return _timeout_closing;
            }
            case TimeoutProcess::Reading: {
                return _timeout_reading;
            }
            case TimeoutProcess::Writing: {
                return _timeout_writing;
            }
        }
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
