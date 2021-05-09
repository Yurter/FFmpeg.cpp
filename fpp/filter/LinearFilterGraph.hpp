#pragma once
#include <fpp/base/FilterGraph.hpp>
#include <fpp/base/Parameters.hpp>
#include <fpp/base/Frame.hpp>

struct AVFilterGraph;

namespace fpp {

class LinearFilterGraph : public FilterGraph {

public:

    LinearFilterGraph(const SpParameters par, const std::vector<std::string>& filters, const Options& options = {});

    FrameVector         filter(const Frame& frame);

};

} // namespace fpp
