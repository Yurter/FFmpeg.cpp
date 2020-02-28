#pragma once
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>
#include <list>

extern "C" {
    #include <libavutil/frame.h>
}

namespace fpp {

    class Frame : public FFmpegObject<AVFrame>, public MediaData {

    public:

        Frame(MediaType type);
        Frame(const Frame& other);
        Frame(const AVFrame& frame, MediaType type);
        virtual ~Frame() override;

        Frame& operator=(const Frame& other);

        int64_t             pts() const;
        void                setPts(int64_t pts);

        bool                keyFrame()  const;

        size_t              size()      const;
        virtual std::string toString()  const override;

    private:

        void                ref(const Frame&   other);
        void                ref(const AVFrame& other);
        void                unref();

    };

    using FrameList = std::list<Frame>;

} // namespace fpp
