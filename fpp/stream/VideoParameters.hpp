#pragma once
#include <fpp/base/Parameters.hpp>

namespace fpp {

    class VideoParameters;
    using SharedVideoParameters = std::shared_ptr<VideoParameters>;

    class VideoParameters : public Parameters {

    public:

        VideoParameters();

        void                setWidth(int width);
        void                setHeight(int height);
        void                setSampleAspectRatio(AVRational sample_aspect_ratio);
        void                setFrameRate(AVRational frame_rate);
        void                setPixelFormat(AVPixelFormat pixel_format);
        void                setGopSize(int gop_size);

        int                 width()         const;
        int                 height()        const;
        int                 gopSize()       const;
        AVRational          frameRate()     const;
        AVPixelFormat       pixelFormat()   const;
        AVRational          sampleAspectRatio() const;

        virtual std::string toString()      const override;
        virtual void        completeFrom(const SharedParameters other)  override;
        virtual bool        betterThen(const SharedParameters& other)   override;
        virtual void        parseStream(const AVStream* avstream)       override;

        static SharedVideoParameters make_shared() {
            return std::make_shared<VideoParameters>();
        }

    private:

        int                 _gop_size;
        AVRational          _frame_rate;

    };

} // namespace fpp
