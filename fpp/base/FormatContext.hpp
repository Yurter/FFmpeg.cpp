#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/core/time/Chronometer.hpp>
#include <fpp/base/Dictionary.hpp>
#include <fpp/stream/Stream.hpp>
#include <array>
#include <chrono>

struct AVFormatContext;
struct AVOutputFormat;
struct AVInputFormat;
struct AVStream;

namespace fpp {

class FormatContext : public SharedFFmpegObject<AVFormatContext> {

public:

    using Timeout = std::chrono::milliseconds;

    enum class TimeoutProcess {
          Opening
        , Closing
        , Reading
        , Writing
        , EnumSize
    };

    FormatContext();

    std::string         mediaResourceLocator() const;
    void                setMediaResourceLocator(const std::string_view mrl);

    unsigned int        streamNumber() const;
    StreamVector        streams();
    const StreamVector  streams() const;
    SharedStream        stream(std::size_t index);
    SharedStream        stream(MediaType stream_type);

    void                setTimeout(TimeoutProcess process, Timeout timout);
    Timeout             getTimeout(TimeoutProcess process) const;

    bool                open(const Options& options = {});
    bool                open(const std::string_view mrl, Options options = {});
    void                close();

    bool                opened() const;
    bool                closed() const;

    void                flushContextAfterEachPacket(bool value);

    std::string         toString() const override final;

protected:

    class Interrupter {

        Chronometer     _chronometer;
        Timeout         _timeout;

    public:

        bool isTimeout() const {
            return _chronometer.elapsed_milliseconds() > _timeout;
        }

        Timeout timeout() const {
            return _timeout;
        }

        void reset(const Timeout& timeout) {
            _timeout = timeout;
            _chronometer.reset();
        }

    };

    void                setInterruptCallback(AVFormatContext* ctx);
    void                setInterruptTimeout(std::chrono::milliseconds timeout);

    virtual void        createContext();
    virtual bool        openContext(const Options& options) = 0;
    virtual void        closeContext() = 0;
    virtual std::string formatName() const = 0;

    void                setFlag(int flag);
    bool                isFlagSet(int flag) const;

    void                addStream(SharedStream stream);
    void                setStreams(StreamVector stream_vector);

    bool                processPacket(Packet& packet);

private:

    void                setOpened(bool opened);
    static int          interrupt_callback(void* opaque);

private:

    std::string         _media_resource_locator; // TODO: use raw()->url instead (05.06)
    bool                _opened;
    StreamVector        _streams;

    using TimeoutsArray = std::array<Timeout,std::size_t(TimeoutProcess::EnumSize)>;
    TimeoutsArray       _timeouts;
    Interrupter         _interrupter;

};

} // namespace fpp
