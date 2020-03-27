#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>
#include <vector>

#define DEFAULT_TIME_BASE AVRational { 1, 1000 }

struct AVStream;

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace fpp {

    class Parameters;
    using SharedParameters = std::shared_ptr<Parameters>;
    using IOParams = struct { SharedParameters in; SharedParameters out; };
    using Extradata = std::vector<uint8_t>;

    class Parameters : public Object, public MediaData {

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

        virtual void        initCodecpar(AVCodecParameters* codecpar) const;
        virtual void        initCodecContext(AVCodecContext* codec_context) const;
        virtual void        parseCodecContext(const AVCodecContext* codec_context);

    private:

        void                setCodec(AVCodec* codec);

    private:

        AVCodec*            _codec;
        AVCodecID           _codec_id;
        unsigned            _codec_tag;
        int64_t             _bitrate;
        int64_t             _duration;
        uid_t               _stream_index;
        AVRational          _time_base;
        Extradata           _extradata;

        int64_t             _bits_per_coded_sample;
        int64_t             _bits_per_raw_sample;
        int                 _profile;
        int                 _level;

    };

} // namespace fpp
