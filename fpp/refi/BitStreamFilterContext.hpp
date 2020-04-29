#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/Parameters.hpp>
#include <fpp/base/Packet.hpp>

struct AVBSFContext;

namespace fpp {

    class BitStreamFilterContext : public SharedFFmpegObject<AVBSFContext> {

    public:

//        BitStreamFilterContext(const InOutParams params, const std::string_view filter_name);
        BitStreamFilterContext(const fpp::SpParameters param, const std::string_view filter_name);

        Packet filter(Packet packet);

    };

} // namespace fpp
