#pragma once
#include <fpp/stream/Stream.hpp>

namespace fpp {

    class OutputStream;
    using SharedOutputStream = std::shared_ptr<OutputStream>;

    class OutputStream : public Stream {

    public:

        OutputStream(AVStream* avstream, const SharedParameters params);
        OutputStream(const SharedParameters parameters);

        static inline SharedOutputStream make_shared(AVStream* avstream, const SharedParameters parameters) {
            return std::make_shared<OutputStream>(avstream, parameters);
        }

        static inline SharedOutputStream make_shared(const SharedParameters parameters) {
            return std::make_shared<OutputStream>(parameters);
        }

    };

} // namespace fpp
