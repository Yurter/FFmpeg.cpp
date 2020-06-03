#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/core/time/Chronometer.hpp>
#include <fpp/base/Dictionary.hpp>
#include <fpp/stream/Stream.hpp>
#include <array>

struct AVFormatContext;
struct AVOutputFormat;
struct AVInputFormat;
struct AVStream;

namespace fpp {

    enum class TimeoutProcess {
          Opening
        , Closing
        , Reading
        , Writing
        , EnumSize
    };

    class FormatContext : public SharedFFmpegObject<AVFormatContext> {

    public:

        FormatContext();

        std::string         mediaResourceLocator() const;
        void                setMediaResourceLocator(const std::string_view mrl);
        const StreamVector  streams() const;
        StreamVector        streams();

        void                setTimeout(TimeoutProcess process, int64_t ms);
        int64_t             getTimeout(TimeoutProcess process) const;

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

        bool                processPacket(Packet& packet);

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
        void                setInterruptTimeout(int64_t timeout_ms);

        virtual void        createContext();
        virtual bool        openContext(Options options) = 0;
        virtual void        closeContext() = 0;
        virtual std::string formatName() const = 0;

        void                addStream(SharedStream stream);
        void                setStreams(StreamVector stream_vector);

    private:

        void                setOpened(bool opened);
        static int          interrupt_callback(void* opaque);

    private:

        std::string         _media_resource_locator;
        bool                _opened;
        StreamVector        _streams;

        using TimeoutsArray = std::array<int64_t,std::size_t(TimeoutProcess::EnumSize)>;
        TimeoutsArray       _timeouts;
        Interrupter         _interrupter;

    };

} // namespace fpp
