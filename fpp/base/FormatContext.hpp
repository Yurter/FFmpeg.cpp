#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/core/time/Chronometer.hpp>
#include <fpp/stream/Stream.hpp>

struct AVFormatContext;
struct AVStream;
struct AVInputFormat;
struct AVOutputFormat;

namespace fpp {

    class FormatContext : public SharedFFmpegObject<AVFormatContext> {

    public:

        FormatContext(const std::string_view mrl);

        std::string         mediaResourceLocator()  const;
        const StreamVector  streams()               const;
        StreamVector        streams();

        void                open();
        void                close();

        bool                opened() const;
        bool                closed() const;

        SharedStream        stream(int64_t index);
        SharedStream        stream(MediaType stream_type);
        int64_t             streamAmount() const;
        void                setStreams(StreamVector stream_list);

        void                processPacket(Packet& packet);

        virtual std::string toString() const override final;

    private:

        /* Операции над формат контестом, ход выполнения
         * которых, отслеживается колбеком                  */
        enum InterruptedProcess {
            None,
            Opening,
            Closing,
            Reading,
            Writing,
        };

        class Interrupter {

        public:

            Interrupter(InterruptedProcess process)
                : interrupted_process(process) {
            }

            InterruptedProcess  interrupted_process;
            Chronometer         chronometer;

        };

    protected:

        virtual void        createContext() = 0;
        virtual void        openContext()   = 0;
        virtual void        closeContext()  = 0;

        [[nodiscard]] virtual StreamVector parseFormatContext() = 0;

        void                addStream(SharedStream stream);

    private:

        void                setOpened(bool opened);
        void                setInteruptCallback(InterruptedProcess process);
        void                resetInteruptCallback();
        static int          interrupt_callback(void* opaque);

    private:

        const std::string   _media_resource_locator;
        bool                _opened;
        StreamVector        _streams;
        Interrupter         _current_interrupter;

    };

} // namespace fpp
