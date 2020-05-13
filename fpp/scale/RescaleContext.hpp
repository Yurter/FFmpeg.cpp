#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/base/Frame.hpp>

struct SwsContext;

namespace fpp {

    class RescaleContext : public SharedFFmpegObject<SwsContext> {

    public:

        explicit RescaleContext(InOutParams parameters);

        Frame               scale(const Frame& frame);

        const InOutParams   params;

    private:

        void                init();
        Frame               createFrame() const;

    };

} // namespace fpp
