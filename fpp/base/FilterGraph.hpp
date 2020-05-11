#pragma once
#include <fpp/base/FilterChain.hpp>
#include <fpp/base/Parameters.hpp>

struct AVFilterGraph;

namespace fpp {

    class FilterGraph : public SharedFFmpegObject<AVFilterGraph> {

    public:

        FilterGraph();

        void                init();

    protected:

        std::string         genUniqueId();

        FilterContext       createBufferSource(const SpParameters par);
        FilterContext       createBufferSink(const SpParameters par);

        std::size_t         emplaceFilterChainBack(FilterChain chain);
        FilterChain&        chain(std::size_t index);

        std::pair<const std::string, const std::string>
        extractNameArgs(const std::string_view filter_descr) const;

        std::vector<FilterContext> createFilterContexts(const SpParameters par, const std::vector<std::string>& filters);

    private:

        using FilterChainVector = std::vector<FilterChain>;

        FilterChainVector   _filters;
        std::size_t         _filter_uid;

    };

} // namespace fpp
