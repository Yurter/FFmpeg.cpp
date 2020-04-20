#pragma once
#include <fpp/base/FilterContext.hpp>

namespace fpp {

    class AudioFilterContext : public FilterContext {

    public:

        std::string         toString() const override;

    private:

        void                initBufferSource()  override;
        void                initBufferSink()    override;

    };

} // namespace fpp
