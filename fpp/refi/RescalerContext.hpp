#pragma once
#include <fpp/base/Frame.hpp>
#include <fpp/stream/VideoParameters.hpp>

extern "C" {
    #include <libswscale/swscale.h>
}

namespace fpp {

    class RescalerContext : public SharedFFmpegObject<SwsContext> {

    public:

        RescalerContext(IOParams parameters);
        virtual ~RescalerContext() override = default;

        Frame               scale(Frame source_frame);

        const IOParams      params;

    private:

        void                init();
        Frame               createFrame() const;

    };

} // namespace fpp
