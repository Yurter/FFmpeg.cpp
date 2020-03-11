#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>

#define DEFAULT_TIME_BASE AVRational { 1, 1000 }

struct AVStream;

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace fpp {

    class Parameters;
    using SharedParameters = std::shared_ptr<Parameters>;
    using IOParams = struct { SharedParameters in; SharedParameters out; };

    class Parameters : public Object, public MediaData {

    public:

        Parameters(MediaType type);
        Parameters(const Parameters&) = default;
        virtual ~Parameters() override = default;

        Parameters& operator=(const Parameters&) = default;

        void                setDecoder(AVCodecID codec_id);
        void                setEncoder(AVCodecID codec_id);

        bool                isDecoder() const;
        bool                isEncoder() const;

        void                setBitrate(int64_t bitrate);
        void                setDuration(int64_t duration);
        void                setStreamIndex(uid_t stream_index);
        void                setTimeBase(AVRational time_base);

        AVCodecID           codecId()       const;
        std::string         codecName()     const;
        AVCodec*            codec()         const;
        int64_t             bitrate()       const;
        int64_t             duration()      const;
        uid_t               streamIndex()   const;
        AVRational          timeBase()      const;

        void                increaseDuration(const int64_t value);

        virtual std::string toString() const override;

        virtual void        completeFrom(const SharedParameters other);
        virtual void        parseStream(const AVStream* avstream);
        virtual void        initStream(AVStream* avstream) const;
        virtual bool        betterThen(const SharedParameters& other);

        static SharedParameters make_shared() {
            return std::make_shared<Parameters>();
        }

    private:

        void                setCodec(AVCodec* codec);

    protected:

        AVCodec*            _codec;
        int64_t             _bitrate;
        int64_t             _duration;
        uid_t               _stream_index;
        AVRational          _time_base;

    };

} // namespace fpp
