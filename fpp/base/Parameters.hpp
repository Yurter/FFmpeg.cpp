#pragma once
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>
#include <vector>
#include <memory>

extern "C" {
    #include <libavcodec/avcodec.h>
}

constexpr auto DEFAULT_TIME_BASE { AVRational { 1, 1000 } };

struct AVStream;
struct AVCodecParams;

namespace fpp {

class Parameters;
using SpParameters = std::shared_ptr<Parameters>;
using InOutParams = struct InOutParams { SpParameters in; SpParameters out; };
using Extradata = std::pair<std::uint8_t*,std::size_t>;

class Parameters : public FFmpegObject<AVCodecParameters>, public Media {

public:

    explicit Parameters(Media::Type type);
    Parameters(const Parameters& other);
    Parameters& operator=(const Parameters& other);

    void                setDecoder(AVCodecID codec_id);
    void                setEncoder(AVCodecID codec_id);

    bool                isDecoder() const;
    bool                isEncoder() const;

    void                setBitrate(std::int64_t bitrate);
    void                setTimeBase(AVRational time_base);
    void                setExtradata(Extradata extradata);
    void                setFormatFlags(int flags);

    AVCodecID           codecId()       const;
    std::string         codecName()     const;
    AVCodec*            codec()         const;
    std::int64_t        bitrate()       const;
    AVRational          timeBase()      const;
    Extradata           extradata()     const;
    std::string         codecType()     const;
    int                 formatFlags()   const;

    std::string         toString() const override;
    virtual bool        betterThen(const SpParameters& other);
    virtual void        completeFrom(const SpParameters other);

    virtual void        parseStream(const AVStream* avstream);

    void                initCodecpar(AVCodecParameters* codecpar) const;
    void                parseCodecpar(AVCodecParameters* codecpar);
    virtual void        initCodecContext(AVCodecContext* codec_context) const;
    virtual void        parseCodecContext(const AVCodecContext* codec_context);

    static SpParameters make_shared(Media::Type media_type) {
        return std::make_shared<Parameters>(media_type);
    }

private:

    void                reset();
    void                setCodec(AVCodec* codec);
    bool                testFormatFlag(int flag) const;

private:

    AVCodec*            _codec;
    AVRational          _time_base;
    int                 _format_flags;

};

} // namespace fpp
