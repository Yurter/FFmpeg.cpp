#pragma once
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>
#include <vector>
#include <memory>

extern "C" {
    #include <libavcodec/avcodec.h> // TODO: move to cpp file (20.04)
}

constexpr auto DEFAULT_TIME_BASE { AVRational { 1, 1000 } };

struct AVStream;
struct AVCodecParams;

namespace fpp {

    class Parameters;
    using SpParameters = std::shared_ptr<Parameters>;
    using InOutParams = struct InOutParams { SpParameters in; SpParameters out; };
    using Extradata = std::pair<uint8_t*,size_t>;

    class Parameters : public FFmpegObject<AVCodecParameters>, public MediaData {

    public:

        Parameters(MediaType type);
        Parameters(const Parameters& other);
        Parameters& operator=(const Parameters& other);

        void                setDecoder(AVCodecID codec_id);
        void                setEncoder(AVCodecID codec_id);

        bool                isDecoder() const;
        bool                isEncoder() const;

        void                setBitrate(int64_t bitrate);
        void                setDuration(int64_t duration);
        void                setStreamIndex(uid_t stream_index);
        void                setTimeBase(AVRational time_base);
        void                setExtradata(Extradata extradata);
        void                setFormatFlags(int flags);

        AVCodecID           codecId()       const;
        std::string         codecName()     const;
        AVCodec*            codec()         const;
        int64_t             bitrate()       const;
        int64_t             duration()      const;
        uid_t               streamIndex()   const;
        AVRational          timeBase()      const;
        Extradata           extradata()     const;
        std::string         codecType()     const;
        int                 formatFlags()   const;

        void                increaseDuration(const int64_t value);

        std::string         toString() const override;
        virtual bool        betterThen(const SpParameters& other);
        virtual void        completeFrom(const SpParameters other);

        virtual void        parseStream(const AVStream* avstream);

        void                initCodecpar(AVCodecParameters* codecpar) const;
        void                parseCodecpar(AVCodecParameters* codecpar);
        virtual void        initCodecContext(AVCodecContext* codec_context) const;
        virtual void        parseCodecContext(const AVCodecContext* codec_context);

        static SpParameters make_shared(MediaType media_type) {
            return std::make_shared<Parameters>(media_type);
        }

    private:

        void                reset();
        void                setCodec(AVCodec* codec);
        bool                testFormatFlag(int flag) const;

    private:

        AVCodec*            _codec;
        int64_t             _duration;      // TODO: remove (24.04)
        uid_t               _stream_index;  // TODO: remove (24.04)
        AVRational          _time_base;
        int                 _format_flags;

    };

} // namespace fpp
