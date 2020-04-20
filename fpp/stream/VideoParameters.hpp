#pragma once
#include <fpp/base/Parameters.hpp>

namespace fpp {

    class VideoParameters;
    using SpVideoParameters = std::shared_ptr<VideoParameters>;

    class VideoParameters : public Parameters {

    public:

        VideoParameters();

        void                setWidth(int width);
        void                setHeight(int height);
        void                setSampleAspectRatio(AVRational sample_aspect_ratio);
        void                setFrameRate(AVRational frame_rate);
        void                setPixelFormat(AVPixelFormat pixel_format);
        void                setGopSize(int gop_size);

        int                 width()             const;
        int                 height()            const;
        int                 gopSize()           const;
        AVRational          frameRate()         const;
        AVPixelFormat       pixelFormat()       const;
        AVRational          sampleAspectRatio() const;

        std::string         toString() const override;
        void                completeFrom(const SpParameters other) override;
        bool                betterThen(const SpParameters& other)  override;
        void                parseStream(const AVStream* avstream)      override;
        void                initCodecContext(AVCodecContext* codec_context) const override;
        void                parseCodecContext(const AVCodecContext* codec_context) override;

        static SpVideoParameters make_shared() {
            return std::make_shared<VideoParameters>();
        }

    private:

        int                 _gop_size;
        AVRational          _frame_rate;

    };

} // namespace fpp
