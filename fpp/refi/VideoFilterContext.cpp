#include "VideoFilterContext.hpp"
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libavfilter/avfilter.h>
    #include <libavutil/opt.h>
}

namespace fpp {

    std::string VideoFilterContext::set_pts(float coef) {
        return "setpts=" + std::to_string(coef) + "*PTS";
    }

    std::string VideoFilterContext::keep_every_frame(int n) {
        return "select='not(mod(n," + std::to_string(n) + "))'";
    }

    void VideoFilterContext::initBufferSource() {
        const AVFilter* buffersrc { ::avfilter_get_by_name("buffer") };
        const auto video_params { std::static_pointer_cast<const VideoParameters>(params) };
        char args[512];

        { /* buffer video source: the decoded frames from the decoder will be inserted here. */
            ::snprintf(args, sizeof(args)
                , "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d"
                , int(video_params->width()), int(video_params->height()), video_params->pixelFormat()
                , video_params->timeBase().num, video_params->timeBase().den
                , video_params->aspectRatio().num, video_params->aspectRatio().den
            );

            AVFilterContext* raw_ptr { nullptr };
            if (const auto ret {
                    ::avfilter_graph_create_filter( /* Создает ~6 потоков */
                        &raw_ptr, buffersrc, "in", args, nullptr, _filter_graph.get()
                )}; ret < 0) {
                throw FFmpegException { "avfilter_graph_create_filter (in) failed", ret };
            }

            _buffersrc_ctx.reset(
                raw_ptr
                , [](auto* ctx) { ::avfilter_free(ctx); }
            );

            log_info("Filter in inited with args: " << args);
        }
    }

    void VideoFilterContext::initBufferSink() {
        const AVFilter* buffersink { ::avfilter_get_by_name("buffersink") };
        const auto video_params { std::static_pointer_cast<const VideoParameters>(params) };
        char args[512];

        { /* buffer video sink: to terminate the filter chain. */
            AVFilterContext* raw_ptr { nullptr };
            if (const auto ret {
                    ::avfilter_graph_create_filter(
                        &raw_ptr, buffersink, "out", nullptr, nullptr, _filter_graph.get()
                )}; ret < 0) {
                throw FFmpegException { "avfilter_graph_create_filter (out) failed", ret };
            }

            _buffersink_ctx.reset(
                raw_ptr
                , [](AVFilterContext* ctx) { ::avfilter_free(ctx); }
            );

            ::snprintf(args, sizeof(args), "NULL");
            log_info("Filter out inited with args: " << args);

            const enum AVPixelFormat pix_fmts[] { video_params->pixelFormat(), AV_PIX_FMT_NONE };

            if (const auto ret {
                    av_opt_set_int_list(
                        _buffersink_ctx.get()
                        , "pix_fmts"
                        , pix_fmts
                        , uint64_t(AV_PIX_FMT_NONE)
                        , AV_OPT_SEARCH_CHILDREN
                )}; ret < 0) {
                throw FFmpegException { CODE_POS + " av_opt_set_int_list failed", ret };
            }
        }
    }

} // namespace fpp
