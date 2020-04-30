#pragma once
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>
#include <vector>

extern "C" {
    #include <libavutil/frame.h>
}

namespace fpp {

    class Frame : public FFmpegObject<AVFrame>, public MediaData {

    public:

        Frame(MediaType type);
        Frame(const Frame& other);
        Frame(const AVFrame& frame, AVRational time_base, MediaType type);
        ~Frame() override;

        Frame& operator=(const Frame& other);

        int64_t             pts() const;
        void                setPts(int64_t pts);
        void                setTimeBase(AVRational time_base);

        AVRational          timeBase()  const;
        bool                keyFrame()  const;
        int                 nbSamples() const;

        size_t              size()      const;
        std::string         toString()  const override;

    private:

        void                ref(const Frame&   other);
        void                ref(const AVFrame& other, AVRational time_base);
        void                unref();

    private:

        AVRational          _time_base;

    };

    using FrameVector = std::vector<Frame>;

} // namespace fpp
