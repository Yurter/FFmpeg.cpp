#pragma once
#include <fpp/base/FilterContext.hpp>

namespace fpp {

    class FilterChain : public Media {

    public:

        explicit FilterChain(Media::Type type);

        void                add(FilterContext ctx);
        void                add(std::vector<FilterContext> ctx_vector);
        void                linkFilters();
        void                linkTo(FilterChain& other);

        FrameVector         read();
        void                write(const Frame& frame);

    private:

        FilterContext&      firstFilter();
        FilterContext&      lastFilter();

    private:

        using Chain = std::vector<FilterContext>;

        Chain               _chain;

    };

} // namespace fpp
