#include "ComplexFilterGraph.hpp"
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/stream/AudioParameters.hpp>

namespace fpp {

    ComplexFilterGraph::ComplexFilterGraph(const Options& options)
        : FilterGraph(options) {
        setName("CompFilterGraph");
    }

    std::size_t ComplexFilterGraph::createInputFilterChain(const SpParameters par, const std::vector<std::string>& filters) {
        FilterChain chain { par->type() };
        chain.add(createBufferSource(par));
        chain.add(createFilterContexts(filters));
        chain.linkFilters();
        return emplaceFilterChainBack(chain);
    }

    std::size_t ComplexFilterGraph::createOutputFilterChain(const SpParameters par, const std::vector<std::string>& filters) {
        FilterChain chain { par->type() };
        chain.add(createFilterContexts(filters));
        chain.add(createBufferSink(par));
        chain.linkFilters();
        return emplaceFilterChainBack(chain);
    }

    std::size_t ComplexFilterGraph::createFilterChain(const SpParameters par, const std::vector<std::string>& filters) {
        FilterChain chain { par->type() };
        chain.add(createFilterContexts(filters));
        chain.linkFilters();
        return emplaceFilterChainBack(chain);
    }

    void ComplexFilterGraph::link(const std::vector<std::size_t>& in, const std::vector<std::size_t>& out) {
        for (const auto& idx_in : in) {
            for (const auto& idx_out : out) {
                chain(idx_in).linkTo(chain(idx_out));
            }
        }
    }

    void ComplexFilterGraph::write(const Frame& frame, std::size_t input_chain_index) {
        chain(input_chain_index).write(frame);
    }

    FrameVector ComplexFilterGraph::read(std::size_t output_chain_index) {
        return chain(output_chain_index).read();
    }

} // namespace fpp
