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
        , _reconnect_on_failure { false } {
        setName("FormatContext");
    }

    void FormatContext::close() {
        if (closed()) {
            return;
        }
        closeContext();
        reset(nullptr);
        setOpened(false);
    }

    bool FormatContext::opened() const {
        return _opened;
    }

    bool FormatContext::closed() const {
        return !_opened;
    }

    void FormatContext::reconnectOnFailure(bool reconnect) {
        _reconnect_on_failure = reconnect;
    }

    void FormatContext::flushContextAfterEachPacket(bool value) {
        raw()->flush_packets = int(value);
    }

    std::string FormatContext::toString() const {
        auto context_info {
            '\n'
            + formatName() + ',' + ' '
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
            log_error("Could not open " + mediaResourceLocator());
            return false;
        }
        setOpened(true);
        log_info(toString());
        return true;
    }

    void FormatContext::setInteruptCallback(AVFormatContext* ctx, Interrupter::Process process, int64_t timeout_ms) {
        _current_interrupter.interrupted_process = process;
        _current_interrupter.chronometer.reset();
        _current_interrupter.timeout_ms = timeout_ms;

        ctx->interrupt_callback.callback = &FormatContext::interrupt_callback;
        ctx->interrupt_callback.opaque   = &_current_interrupter;
    }

    void FormatContext::resetInteruptCallback() {
        _current_interrupter.interrupted_process = Interrupter::Process::None;
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
        static_log_warning("inter", "--------" << interrupter->interrupted_process);
        if (interrupter->isNone()) {
            return OK;
        }
        if (interrupter->isTimeout()) {
            static_log_error(
                "interrupt_callback"
                , interrupter->interrupted_process
                    << " timed out: "
                    << interrupter->timeout_ms
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

    int64_t FormatContext::streamNumber() const {
        return int64_t(raw()->nb_streams);
    }

    StreamVector FormatContext::streams() {
        return _streams;
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
