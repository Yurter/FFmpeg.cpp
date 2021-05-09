#pragma once
#include <fpp/base/FilterGraph.hpp>
#include <fpp/base/Parameters.hpp>
#include <fpp/base/Frame.hpp>

struct AVFilterGraph;

namespace fpp {

class ComplexFilterGraph : public FilterGraph {

public:

    explicit ComplexFilterGraph(const Options& options = {});

    std::size_t         createInputFilterChain (const SpParameters par, const std::vector<std::string>& filters);
    std::size_t         createOutputFilterChain(const SpParameters par, const std::vector<std::string>& filters);
    std::size_t         createFilterChain(const SpParameters par, const std::vector<std::string>& filters);

    void                link(const std::vector<std::size_t>& in, const std::vector<std::size_t>& out);

    void                write(const Frame& frame, std::size_t input_chain_index);
    FrameVector         read(std::size_t output_chain_index);

};

} // namespace fpp
