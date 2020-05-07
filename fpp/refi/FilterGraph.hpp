#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/FilterContext.hpp>
#include <fpp/base/Parameters.hpp>
#include <fpp/base/Frame.hpp>

//struct AVFilterGraph;
//struct AVFilterContext;
//struct AVFilter;

namespace fpp {

    using FilterChain = struct FilterChain {
        FilterChain(MediaType mt, AVRational tb)
            : type { mt }, time_base { tb } {}
        MediaType type;
        AVRational time_base;
        std::vector<FilterContext> filters;
    };
    using FilterChainVector = std::vector<FilterChain>;

    class FilterGraph : SharedFFmpegObject<AVFilterGraph> {

    public:

        FilterGraph();

        std::size_t         createInputFilterChain(const SpParameters par, std::vector<std::string> filters);
        std::size_t         createOutputFilterChain(const SpParameters par, std::vector<std::string> filters);
        void                createSimpleFilterChain(const SpParameters par, std::vector<std::string> filters);

        void                link(std::vector<std::size_t> in, std::vector<std::size_t> out);

        void                write(const Frame& frame, std::size_t input_chain_index = 0);
        FrameVector         read(std::size_t output_chain_index = 0);

    private:

        std::string         genUniqueName();

        FilterContext       createBufferSource(const SpParameters par);
        FilterContext       createBufferSink(const SpParameters par);

        void                linkChain(fpp::FilterChain& chain);
        std::size_t         emplaceFilterChainBack(FilterChain&& chain);

        std::pair<const std::string, const std::string> splitFilterDescription(const std::string_view filter_descr) const;

    private:

        FilterChainVector   _filters;
        std::size_t         _filter_uid;

    };


} // namespace fpp
