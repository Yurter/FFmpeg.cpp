#include "FormatContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavdevice/avdevice.h>
}

namespace fpp {

    FormatContext::FormatContext(const std::string_view mrl)
        : _media_resource_locator { mrl }
        , _opened { false }
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
        setInterrupter(timeoutClosing());
        closeContext();
        reset(nullptr);
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
        auto context_info {
            formatName() + ',' + ' '
            + mediaResourceLocator() + ':'
        };
        for (const auto& stream : streams()) {
            context_info += '\n' + stream->toString();
        }
        return context_info;
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
            log_error("Could not open ", mediaResourceLocator());
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

    void FormatContext::setInterrupter(int64_t timeout_ms) {
        _interrupter.set(timeout_ms);
    }

    void FormatContext::createContext() {
        reset(std::shared_ptr<AVFormatContext> {
            ::avformat_alloc_context()
            , [](auto* ctx) { ::avformat_free_context(ctx); }
        });
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

    std::string FormatContext::mediaResourceLocator() const {
        return _media_resource_locator;
    }

    void FormatContext::setStreams(StreamVector stream_list) {
        _streams = stream_list;
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

    void FormatContext::setMediaResourceLocator(const std::string_view mrl) {
        _media_resource_locator = mrl;
    }

    int64_t FormatContext::streamNumber() const {
        return int64_t(raw()->nb_streams);
    }

    StreamVector FormatContext::streams() {
        return _streams;
    }

    void FormatContext::setTimeoutOpening(int64_t ms) {
        _timeout_opening = ms;
    }

    void FormatContext::setTimeoutClosing(int64_t ms) {
        _timeout_closing = ms;
    }

    void FormatContext::setTimeoutReading(int64_t ms) {
        _timeout_reading = ms;
    }

    void FormatContext::setTimeoutWriting(int64_t ms) {
        _timeout_writing = ms;
    }

    int64_t FormatContext::timeoutOpening() const {
        return _timeout_opening;
    }

    int64_t FormatContext::timeoutClosing() const {
        return _timeout_closing;
    }

    int64_t FormatContext::timeoutReading() const {
        return _timeout_reading;
    }

    int64_t FormatContext::timeoutWriting() const {
        return _timeout_writing;
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
