#pragma once
#include <fpp/base/FilterContext.hpp>

namespace fpp {

    class VideoFilterContext : public FilterContext {

    public:

        VideoFilterContext(SpParameters parameters, const std::string/*_view*/ filters_descr);

        static std::string  set_pts(float coef);
        static std::string  keep_every_frame(int n);

    protected:

        void                initBufferSource()  override;
        void                initBufferSink()    override;

    };

} // namespace fpp
