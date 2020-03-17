#pragma once
#include <fpp/base/Frame.hpp>
#include <fpp/stream/VideoParameters.hpp>

extern "C" {
    #include <libswscale/swscale.h>
}

namespace fpp {

    class RescaleContext : public SharedFFmpegObject<SwsContext> {

    public:

        RescaleContext(IOParams parameters);
        virtual ~RescaleContext() override = default;

        Frame               scale(const Frame source_frame);

        const IOParams      params;

    private:

        void                init();
        Frame               createFrame() const;

    };

} // namespace fpp
