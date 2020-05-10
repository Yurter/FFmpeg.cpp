#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/stream/AudioParameters.hpp>
#include <fpp/base/Frame.hpp>

struct SwrContext;

namespace fpp {

    class ResampleContext : public SharedFFmpegObject<SwrContext> {

    public:

        explicit ResampleContext(InOutParams parameters);

        FrameVector         resample(const Frame& frame);

        const InOutParams   params;

    private:

        void                init();
        Frame               createFrame() const;
        void                sendFrame(const Frame& frame);
        FrameVector         receiveFrames(AVRational time_base, int stream_index);
        void                stampFrame(Frame& frame);

    private:

        int64_t             _samples_count;
        int64_t             _source_pts;

    };

} // namespace fpp
