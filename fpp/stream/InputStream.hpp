#pragma once
#include <fpp/stream/Stream.hpp>

namespace fpp {

    class InputStream;
    using SharedInputStream = std::shared_ptr<InputStream>;

    class InputStream : public Stream {

    public:

        InputStream(AVStream* avstream);
        InputStream(const SharedParameters parameters);

        static inline SharedInputStream make_shared(AVStream* avstream) {
            return std::make_shared<InputStream>(avstream);
        }

        static inline SharedInputStream make_shared(const SharedParameters parameters) {
            return std::make_shared<InputStream>(parameters);
        }

    };

} // namespace fpp
