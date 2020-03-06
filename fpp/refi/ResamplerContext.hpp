#pragma once
#include <fpp/base/Frame.hpp>
#include <fpp/stream/AudioParameters.hpp>

extern "C" {
    #include <libswresample/swresample.h>
}

namespace fpp {

    class ResamplerContext : public SharedFFmpegObject<SwrContext> {

    public:

        ResamplerContext(IOParams parameters);
        virtual ~ResamplerContext() override = default;

        FrameList           resample(const Frame source_frame);

        const IOParams      params;

    private:

        void                init();
        Frame               createFrame() const;

    };

} // namespace fpp
