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
        , _current_interrupter { Interrupter { InterruptedProcess::None } } {
        setName("FormatContext");
    }

    void FormatContext::close() {
        if (closed()) {
            return;
        }
        closeContext();
        setOpened(false);
    }

    bool FormatContext::opened() const {
        return _opened;
    }

    bool FormatContext::closed() const {
        return !_opened;
    }

    std::string FormatContext::toString() const {
        std::string context_info { "'" + mediaResourceLocator() + "'," };
        for (const auto& stream : streams()) {
            context_info += '\n' + stream->toString();
        }
        if (context_info.back() == ',') {
            context_info += "without streams";
        }
        return context_info;
    }

    void FormatContext::beforeCloseContext() {
        //
    }

    void FormatContext::setOpened(bool opened) {
        _opened = opened;
    }

    void FormatContext::open() {
        if (opened()) {
            throw std::runtime_error { "Context already opened" };
        }
//        if (streamAmount() == 0) { потоков у инпута нет до открытия
//            throw std::logic_error { "Can't open context without streams" };
//        }
        setInteruptCallback(InterruptedProcess::Opening);
        openContext();
        resetInteruptCallback();
        setOpened(true);
    }

    void FormatContext::setInteruptCallback(InterruptedProcess process) {
        _current_interrupter.interrupted_process = process;
        _current_interrupter.chronometer.reset_timepoint();
        raw()->interrupt_callback.callback =
            &FormatContext::interrupt_callback;
        raw()->interrupt_callback.opaque =
            &_current_interrupter;
    }

    void FormatContext::resetInteruptCallback() {
        _current_interrupter.interrupted_process = InterruptedProcess::None;
        raw()->interrupt_callback.callback = nullptr; /* Бессмысленно :( */
        raw()->interrupt_callback.opaque   = nullptr; /* Бессмысленно :( */
    }

    int FormatContext::interrupt_callback(void* opaque) {
        const auto interrupter { reinterpret_cast<const Interrupter*>(opaque) };
        switch (interrupter->interrupted_process) {
        case InterruptedProcess::None:
            return 0;
        case InterruptedProcess::Opening: {
            const int64_t opening_timeout_ms = 10'000;
            if (interrupter->chronometer.elapsed_milliseconds() > opening_timeout_ms) {
                static_log_error("interrupt_callback", "Opening timed out: " << opening_timeout_ms);
                return 1;
            }
            return 0;
        }
        case InterruptedProcess::Closing:
            throw std::runtime_error { "InterruptedProcess::Closing is not implemeted" };
        case InterruptedProcess::Reading:
            throw std::runtime_error { "InterruptedProcess::Reading is not implemeted" };
        case InterruptedProcess::Writing:
            throw std::runtime_error { "InterruptedProcess::Writing is not implemeted" };
        }
        throw std::invalid_argument { "Bad InterruptedProcess arg" };
    }

    void FormatContext::closeContext() {
        beforeCloseContext();
        if (const auto ret { ::avio_close(raw()->pb) }; ret < 0) {
            throw FFmpegException {
                "Failed to close " + mediaResourceLocator()
                , ret
            };
        }
    }

    std::string FormatContext::mediaResourceLocator() const {
        return _media_resource_locator;
    }

    void FormatContext::setStreams(StreamVector stream_list) {
        _streams = stream_list;
    }

    void FormatContext::processPacket(Packet& packet) {
        const auto packet_stream { stream(packet.streamIndex()) };
        packet_stream->stampPacket(packet);
        const auto packet_type {
            packet_stream->timeIsOver()
            ? MediaType::EndOF
            : packet_stream->params->type()
        };
        packet.setType(packet_type);
    }

    void FormatContext::addStream(SharedStream stream) {
        _streams.push_back(stream);
    }

    int64_t FormatContext::streamAmount() const {
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
