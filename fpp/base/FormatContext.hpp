#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/core/time/Chronometer.hpp>
#include <fpp/base/Dictionary.hpp>
#include <fpp/stream/Stream.hpp>

struct AVFormatContext;
struct AVOutputFormat;
struct AVInputFormat;
struct AVStream;

namespace fpp {

    class FormatContext : public SharedFFmpegObject<AVFormatContext> {

    public:

        FormatContext(const std::string_view mrl = {});

        std::string         mediaResourceLocator()  const;
        const StreamVector  streams()               const;
        StreamVector        streams();

        void                setTimeoutOpening(int64_t ms);
        void                setTimeoutClosing(int64_t ms);
        void                setTimeoutReading(int64_t ms);
        void                setTimeoutWriting(int64_t ms);

        int64_t             timeoutOpening() const;
        int64_t             timeoutClosing() const;
        int64_t             timeoutReading() const;
        int64_t             timeoutWriting() const;

        bool                open(Options options = {});
        bool                open(const std::string_view mrl, Options options = {});
        void                close();

        bool                opened() const;
        bool                closed() const;

        void                reconnectOnFailure(bool reconnect);
        void                flushContextAfterEachPacket(bool value);

        SharedStream        stream(int64_t index);
        SharedStream        stream(MediaType stream_type);
        int64_t             streamNumber() const;
        void                setStreams(StreamVector stream_list); // TODO remove 06.04

        void                processPacket(Packet& packet);

        std::string         toString() const override final;

    protected:

        struct Interrupter {

            enum Process {
                None, // TODO remove 13.04
                Opening,
                Closing,
                Reading,
                Writing,
            };

            Process         interrupted_process { Process::None };
            Chronometer     chronometer;
            int64_t         timeout_ms { 0 };

            bool isNone() const {
                return interrupted_process == Process::None;
            }

            bool isTimeout() const {
                return chronometer.elapsed_milliseconds() > timeout_ms;
            }

            void reset() {
                interrupted_process = Process::None;
            }

        };

        void                setInteruptCallback(AVFormatContext* ctx, Interrupter::Process process, int64_t timeout_ms);
        void                resetInteruptCallback();

        virtual void        createContext();
        virtual bool        openContext(Options options) = 0;
        virtual void        closeContext() = 0;
        virtual std::string formatName() const = 0;

        [[nodiscard]]
        virtual StreamVector parseFormatContext() = 0;

        void                addStream(SharedStream stream);

    private:

        void                setMediaResourceLocator(const std::string_view mrl);
        void                setOpened(bool opened);
        static int          interrupt_callback(void* opaque);

    private:

        std::string         _media_resource_locator;
        bool                _opened;
        StreamVector        _streams;
        Interrupter         _current_interrupter;

        int64_t             _timeout_opening;
        int64_t             _timeout_closing; // TODO not used 10.04
        int64_t             _timeout_reading; // TODO not used 10.04
        int64_t             _timeout_writing; // TODO not used 10.04

    };

} // namespace fpp
