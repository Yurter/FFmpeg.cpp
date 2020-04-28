#pragma once
#include <fpp/base/CodecContext.hpp>

namespace fpp {

    class BitStreamFilterContext : public SharedFFmpegObject<AVBSFContext> {

    public:

        BitStreamFilterContext(const std::string_view filter_name);

        Packet filter(Packet packet);

    };

} // namespace fpp
