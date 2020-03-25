#pragma once
#include <fpp/base/FilterContext.hpp>

namespace fpp {

    class AudioFilterContext : public FilterContext {

    public:

        virtual std::string toString() const override;

    private:

        virtual void        initBufferSource()  override;
        virtual void        initBufferSink()    override;

    };

} // namespace fpp
