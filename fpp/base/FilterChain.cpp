#include "FilterChain.hpp"

namespace fpp {

    FilterChain::FilterChain(fpp::MediaType type)
        : MediaData(type) {
    }

    void FilterChain::add(FilterContext ctx) {
        _chain.push_back(ctx);
    }

    void FilterChain::add(std::vector<FilterContext> ctx_vector) {
        for (const auto& ctx : ctx_vector) {
            add(ctx);
        }
    }

    void FilterChain::linkFilters() {
        for (std::size_t i { 0 }; i < _chain.size() - 1; ++i) {
            _chain.at(i).linkTo(_chain.at(i+1));
        }
    }

    void FilterChain::linkTo(FilterChain& other) {
        lastFilter().linkTo(other.firstFilter());
    }

    FrameVector FilterChain::read() {
        auto filtered_frames { lastFilter().read() };
        for (auto& frame : filtered_frames) {
            frame.setType(type());
        }
        return filtered_frames;
    }

    void FilterChain::write(const Frame& frame) {
        firstFilter().write(frame);
    }

    FilterContext& FilterChain::firstFilter() {
        return _chain.front();
    }

    FilterContext& FilterChain::lastFilter() {
        return _chain.back();
    }

} // namespace fpp
