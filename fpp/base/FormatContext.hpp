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

        FormatContext();

        std::string         mediaResourceLocator()  const;
        void                setMediaResourceLocator(const std::string_view);
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

        void                processPacket(Packet& packet);

        std::string         toString() const override final;

    protected:

        struct Interrupter {

            Chronometer     chronometer;
            int64_t         timeout_ms { 0 };

            bool isTimeout() const {
                return chronometer.elapsed_milliseconds() > timeout_ms;
            }

            void set(int64_t timeout) {
                timeout_ms = timeout;
                chronometer.reset();
            }

        };

        void                setInterruptCallback(AVFormatContext* ctx);
        void                setInterrupter(int64_t timeout_ms);

        virtual void        createContext();
        virtual bool        openContext(Options options) = 0;
        virtual void        closeContext() = 0;
        virtual std::string formatName() const = 0;

        [[nodiscard]]
        virtual StreamVector parseFormatContext() = 0;

        void                addStream(SharedStream stream);
        void                setStreams(StreamVector stream_vector);

    private:

        void                setOpened(bool opened);
        static int          interrupt_callback(void* opaque);

    private:

        std::string         _media_resource_locator;
        bool                _opened;
        StreamVector        _streams;
        Interrupter         _interrupter;

        int64_t             _timeout_opening;
        int64_t             _timeout_closing;
        int64_t             _timeout_reading;
        int64_t             _timeout_writing;

    };

} // namespace fpp
