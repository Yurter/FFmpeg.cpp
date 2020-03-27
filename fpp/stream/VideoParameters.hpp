#pragma once
#include <fpp/base/Parameters.hpp>

namespace fpp {

    class VideoParameters;
    using SharedVideoParameters = std::shared_ptr<VideoParameters>;

    class VideoParameters : public Parameters {

    public:

        VideoParameters();
        virtual ~VideoParameters() override = default;

        void                setWidth(int64_t width);
        void                setHeight(int64_t height);
        void                setSampleAspectRatio(AVRational sample_aspect_ratio);
        void                setFrameRate(AVRational frame_rate);
        void                setPixelFormat(AVPixelFormat pixel_format);
        void                setGopSize(int64_t gop_size);

        int64_t             width()         const;
        int64_t             height()        const;
        int64_t             gopSize()       const;
        AVRational          frameRate()     const;
        AVPixelFormat       pixelFormat()   const;
        AVRational          sampleAspectRatio() const;

        virtual std::string toString()      const override;
        virtual void        completeFrom(const SharedParameters other)  override;
        virtual bool        betterThen(const SharedParameters& other)   override;
        virtual void        parseStream(const AVStream* avstream)       override;

        virtual void        initCodecContext(AVCodecContext* codec_context) const override;
        virtual void        parseCodecContext(const AVCodecContext* codec_context) override;

        virtual void        initCodecpar(AVCodecParameters* codecpar) const override;

        static SharedVideoParameters make_shared() {
            return std::make_shared<VideoParameters>();
        }

    private:

        int64_t             _width;
        int64_t             _height;
        AVRational          _sample_aspect_ratio;
        AVRational          _frame_rate;
        AVPixelFormat       _pixel_format;
        int64_t             _gop_size;

        AVFieldOrder        _field_order;
        AVColorRange        _color_range;
        AVColorPrimaries    _color_primaries;
        AVColorSpace        _color_space;
        AVChromaLocation    _chroma_location;
        int64_t             _video_delay;
        AVColorTransferCharacteristic _color_trc;

    };

} // namespace fpp
