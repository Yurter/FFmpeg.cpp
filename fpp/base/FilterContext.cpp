#include "FilterContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavfilter/avfilter.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
    #include <libavutil/opt.h>
}

namespace fpp {

    FilterContext::FilterContext(MediaType type
                                 , AVRational time_base
                                 , AVFilterGraph* graph
                                 , const std::string_view name
                                 , const std::string_view unique_name
                                 , const std::string_view args
                                 , void* opaque)
        : _type { type }
        , _time_base { time_base }
        , _nb_input_pads { 0 }
        , _nb_output_pads { 0 } {

        setName("FilterContext");
        AVFilterContext* flt_ctx { nullptr };
        ffmpeg_api_strict(avfilter_graph_create_filter
            , &flt_ctx
            , getFilterByName(name)
//            , unique_name.data()
            , (std::string { name } + '_' + unique_name.data()).c_str()
            , args.data()
            , opaque
            , graph
        );
        reset({
            flt_ctx
            , [](auto* ctx) { ::avfilter_free(ctx); }
        });

    }

    void FilterContext::linkTo(FilterContext& other) {
        ffmpeg_api_strict(avfilter_link
            , raw()
            , _nb_input_pads++
            , other.raw()
            , other._nb_output_pads++
        );
    }

    FrameVector FilterContext::read() {
        FrameVector filtered_frames;
        auto ret { 0 };
        while (ret == 0) {
            Frame output_frame { _type };
            log_warning(raw()->name);
            ret = ::av_buffersink_get_frame(raw(), output_frame.ptr());
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                throw FFmpegException { "av_buffersink_get_frame failed", ret };
            }
            output_frame.setTimeBase(_time_base);
            filtered_frames.push_back(output_frame);
        }
        return filtered_frames;
    }

    void FilterContext::write(const Frame& frame) {
        ffmpeg_api_strict(av_buffersrc_write_frame
           , raw()
           , frame.ptr()
        );
    }

    const AVFilter* FilterContext::getFilterByName(const std::string_view name) const {
        const auto filter {
            ::avfilter_get_by_name(name.data())
        };
        if (!filter) {
            throw FFmpegException {
                "Failed to found " + std::string { name } + " filter!"
            };
        }
        return filter;
    }

} // namespace fpp
