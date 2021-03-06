#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/Frame.hpp>

struct AVFilter;
struct AVFilterContext;
struct AVFilterGraph;

namespace fpp {

class FilterContext : public SharedFFmpegObject<AVFilterContext> {

public:

    FilterContext(AVFilterGraph* graph
                , const std::string_view name
                , const std::string_view unique_id
                , const std::string_view args
                , void* opaque);

    void                linkTo(FilterContext& other);
    void                setAudioBufferSinkFrameSize(unsigned frame_size);

    FrameVector         read();
    void                write(const Frame& frame);

private:

    const AVFilter*     getFilterByName(const std::string_view name) const;

private:

    unsigned            _nb_input_pads;
    unsigned            _nb_output_pads;

};

} // namespace fpp
