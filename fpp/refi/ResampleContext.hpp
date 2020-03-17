#pragma once
#include <fpp/base/Frame.hpp>
#include <fpp/stream/AudioParameters.hpp>

extern "C" {
    #include <libswresample/swresample.h>
}

namespace fpp {

    class ResampleContext : public SharedFFmpegObject<SwrContext> {

    public:

        ResampleContext(IOParams parameters);
        virtual ~ResampleContext() override = default;

        FrameList           resample(const Frame source_frame);

        const IOParams      params;

    private:

        void                init();
        Frame               createFrame() const;
        void                sendFrame(const Frame source_frame);
        FrameList           receiveFrames();
        void                stampFrame(Frame& output_frame);

    private:

        int64_t             _samples_count;
        int64_t             _source_pts;

    };

} // namespace fpp
