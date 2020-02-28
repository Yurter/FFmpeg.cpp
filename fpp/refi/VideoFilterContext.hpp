#pragma once
#include <fpp/refi/FilterContext.hpp>

namespace fpp {

    class VideoFilterContext : public FilterContext {

    public:

        static std::string  set_pts(float coef);
        static std::string  keep_every_frame(int n);


        virtual std::string toString() const override;

    private:

        virtual void        initBufferSource()  override;
        virtual void        initBufferSink()    override;

    };

} // namespace fpp
