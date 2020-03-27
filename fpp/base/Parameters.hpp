#pragma once
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>
#include <vector>

#define DEFAULT_TIME_BASE AVRational { 1, 1000 }

struct AVStream;
struct AVCodecParams;

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace fpp {

    class Parameters;
    using SharedParameters = std::shared_ptr<Parameters>;
    using IOParams = struct { SharedParameters in; SharedParameters out; };
    using Extradata = std::pair<uint8_t*,size_t>;

    class Parameters : public FFmpegObject<AVCodecParameters>, public MediaData {

    public:

        Parameters(MediaType type);

        void                setDecoder(AVCodecID codec_id);
        void                setEncoder(AVCodecID codec_id);

        bool                isDecoder() const;
        bool                isEncoder() const;

        void                setBitrate(int64_t bitrate);
        void                setDuration(int64_t duration);
        void                setStreamIndex(uid_t stream_index);
        void                setTimeBase(AVRational time_base);
        void                setExtradata(Extradata extradata);

        AVCodecID           codecId()       const;
        std::string         codecName()     const;
        AVCodec*            codec()         const;
        int64_t             bitrate()       const;
        int64_t             duration()      const;
        uid_t               streamIndex()   const;
        AVRational          timeBase()      const;
        Extradata           extradata()     const;
        std::string         codecType()     const;

        void                increaseDuration(const int64_t value);

        virtual std::string toString() const override;
        virtual bool        betterThen(const SharedParameters& other);
        virtual void        completeFrom(const SharedParameters other);

        virtual void        parseStream(const AVStream* avstream);

        void                initCodecpar(AVCodecParameters* codecpar) const;
        void                parseCodecpar(AVCodecParameters* codecpar);
        void                initCodecContext(AVCodecContext* codec_context) const;
        void                parseCodecContext(const AVCodecContext* codec_context);

    private:

        void                reset();
        void                setCodec(AVCodec* codec);

    private:

        AVCodec*            _codec;
        int64_t             _duration;
        uid_t               _stream_index;
        AVRational          _time_base;

    };

} // namespace fpp
