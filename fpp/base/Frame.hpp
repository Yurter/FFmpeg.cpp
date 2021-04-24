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

        explicit Frame(MediaType type = MediaType::Unknown);
        Frame(const Frame& other);
        Frame(const AVFrame& frame, MediaType type, AVRational time_base, int stream_index);
        ~Frame() override;

        Frame& operator=(const Frame& other);

        std::int64_t        pts() const;
        void                setPts(std::int64_t pts);
        void                setTimeBase(AVRational time_base);
        void                setStreamIndex(int stream_index);

        AVRational          timeBase()  const;
        int                 streamIndex()  const;
        bool                keyFrame()  const;
        int                 nbSamples() const;

        int                 size()      const;
        std::string         toString()  const override;

    private:

        void                ref(const Frame&   other);
        void                ref(const AVFrame& other, AVRational time_base, int stream_index);
        void                unref();

    private:

        AVRational          _time_base;
        int                 _stream_index;

    };

    using FrameVector = std::vector<Frame>;

} // namespace fpp
