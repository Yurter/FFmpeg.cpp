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

        FormatContext(const std::string_view mrl);

        std::string         mediaResourceLocator()  const;
        const StreamVector  streams()               const;
        StreamVector        streams();

        bool                open(Options options = {});
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

        virtual std::string toString() const override final;

//    private:

        /* Операции над формат контестом, ход выполнения
         * которых, отслеживается колбеком                  */
        enum InterruptedProcess {
            None,
            Opening,
            Closing,
            Reading,
            Writing,
        };

        struct Interrupter {

            InterruptedProcess  interrupted_process;
            Chronometer         chronometer;
            int64_t             timeout_ms;

            bool isTimeout() const {
                return chronometer.elapsed_milliseconds() > timeout_ms;
            }

        };

    protected:

        virtual void        createContext() = 0;
        virtual bool        openContext(Options options) = 0;
        virtual void        beforeCloseContext();
        virtual std::string formatName() const = 0;

        [[nodiscard]]
        virtual StreamVector parseFormatContext() = 0;

        void                addStream(SharedStream stream);

    private:

        void                setOpened(bool opened);
        void                setInteruptCallback(AVFormatContext* ctx, InterruptedProcess process, int64_t timeout_ms);
        void                resetInteruptCallback(AVFormatContext* ctx);
        static int          interrupt_callback(void* opaque);
        void                closeContext();

    private:

        const std::string   _media_resource_locator;
        bool                _opened;
        StreamVector        _streams;
        Interrupter         _current_interrupter;
        bool                _reconnect_on_failure;

    };

} // namespace fpp
