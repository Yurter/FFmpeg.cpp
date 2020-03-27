#include "VideoFilterContext.hpp"
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libavfilter/avfilter.h>
    #include <libavutil/opt.h>
}

namespace fpp {

    VideoFilterContext::VideoFilterContext(SharedParameters parameters, const std::string/*_view*/ filters_descr)
        : FilterContext(parameters, filters_descr) {
        setName("VideoFltCtx");
        init();
    }

    std::string VideoFilterContext::set_pts(float coef) {
        return "setpts=" + std::to_string(coef) + "*PTS";
    }

    std::string VideoFilterContext::keep_every_frame(int n) {
        return "select='not(mod(n," + std::to_string(n) + "))'";
    }

    std::string VideoFilterContext::toString() const {
        return std::string { "TODO 23.03" };
    }

    void VideoFilterContext::initBufferSource() {
        const auto buffersrc {
            ::avfilter_get_by_name("buffer")
        };
        const auto video_params {
            std::static_pointer_cast<const VideoParameters>(params)
        };
        char args[512];

        /* buffer video source: the decoded frames from the decoder
         * will be inserted here */
        ::snprintf(args, sizeof(args)
            , "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d"
            , int(video_params->width())
            , int(video_params->height())
            , video_params->pixelFormat()
            , video_params->timeBase().num
            , video_params->timeBase().den
            , video_params->sampleAspectRatio().num
            , video_params->sampleAspectRatio().den
        );

        AVFilterContext* raw_ptr { nullptr };
        if (const auto ret {
                ::avfilter_graph_create_filter(
                    &raw_ptr
                    , buffersrc
                    , "in"      /* name   */
                    , args
                    , nullptr   /* opaque */
                    , _filter_graph.get()
            )}; ret < 0) {
            throw FFmpegException {
                __FUNCTION__ " avfilter_graph_create_filter() failed"
                , ret
            };
        }

        _buffersrc_ctx.reset(
            raw_ptr
            , [](auto* ctx) { ::avfilter_free(ctx); }
        );

        log_debug("Filter in inited with args: " << args);
    }

    void VideoFilterContext::initBufferSink() {
        const auto buffersink {
            ::avfilter_get_by_name("buffersink")
        };
        const auto video_params {
            std::static_pointer_cast<const VideoParameters>(params)
        };

        /* buffer video sink: to terminate the filter chain */
        AVFilterContext* raw_ptr { nullptr };
        if (const auto ret {
                ::avfilter_graph_create_filter(
                    &raw_ptr
                    , buffersink
                    , "out"     /* name   */
                    , nullptr   /* args   */
                    , nullptr   /* opaque */
                    , _filter_graph.get()
            )}; ret < 0) {
            throw FFmpegException {
                __FUNCTION__ " avfilter_graph_create_filter() failed"
                , ret
            };
        }

        _buffersink_ctx.reset(
            raw_ptr
            , [](auto* ctx) { ::avfilter_free(ctx); }
        );

        const AVPixelFormat pix_fmts[] {
            video_params->pixelFormat()
            , AV_PIX_FMT_NONE
        };

        const auto array_size {
            int(
                ::av_int_list_length_for_size(
                    sizeof(*(pix_fmts))
                    , pix_fmts
                    , uint64_t(AV_PIX_FMT_NONE)
                )
                * sizeof(*(pix_fmts))
            )
        };

        if (const auto ret {
            ::av_opt_set_bin(
                _buffersink_ctx.get()
                , "pix_fmts"
                ,  reinterpret_cast<const uint8_t*>(pix_fmts)
                , array_size
                , AV_OPT_SEARCH_CHILDREN
        )}; ret < 0) {
            throw FFmpegException {
                __FUNCTION__ " av_opt_set_bin failed"
                , ret
            };
        }

    }

} // namespace fpp










