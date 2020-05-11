#include "LinearFilterGraph.hpp"

namespace fpp {

    LinearFilterGraph::LinearFilterGraph(const SpParameters par, const std::vector<std::string>& filters, const Options& options)
        : FilterGraph(options) {
        setName("LineFilterGraph");
        FilterChain chain { par->type() };
        chain.add(createBufferSource(par));
        chain.add(createFilterContexts(filters));
        chain.add(createBufferSink(par));
        chain.linkFilters();
        emplaceFilterChainBack(chain);
    }

    FrameVector LinearFilterGraph::filter(const Frame& frame) {
        chain(0).write(frame);
        return chain(0).read();
    }

} // namespace fpp
