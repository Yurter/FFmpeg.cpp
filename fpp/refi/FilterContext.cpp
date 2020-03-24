#include "FilterContext.hpp"
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavfilter/avfilter.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
    #include <libavutil/opt.h>
}

namespace fpp {

    FilterContext::FilterContext(SharedParameters parameters, const std::string& filters_descr)
        : params(parameters)
        , _filters_descr(filters_descr) {
        setName("FilterContext");
    }

    FrameList FilterContext::filter(Frame source_frame) {
        if (const auto ret {
                ::av_buffersrc_add_frame_flags(
                    _buffersrc_ctx.get()
                    , source_frame.ptr()
                    , AV_BUFFERSRC_FLAG_KEEP_REF
                )
            }; ret < 0) {
            throw FFmpegException { "av_buffersrc_add_frame_flags failed", ret };
        }
        FrameList filtered_frames;
        /* pull filtered frames from the filtergraph */
        auto ret { 0 };
        while (ret == 0) {
            Frame output_frame { params->type() };
            ret = ::av_buffersink_get_frame(_buffersink_ctx.get(), output_frame.ptr());
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                throw FFmpegException { "av_buffersink_get_frame failed", ret };
            }
            output_frame.setTimeBase(params->timeBase());
            filtered_frames.push_back(output_frame);
        }
        return filtered_frames;
    }

    std::string FilterContext::description() const {
        return _filters_descr;
    }

    void FilterContext::init() {
        auto inputs {
            std::shared_ptr<AVFilterInOut> {
                ::avfilter_inout_alloc()
                , [](auto* filter) { /*::avfilter_inout_free(&filter);*/ } // TODO: приходит мусор, падение 04.02
            }
        };

        auto outputs {
            std::shared_ptr<AVFilterInOut> {
                ::avfilter_inout_alloc()
                , [](auto* filter) { /*::avfilter_inout_free(&filter);*/ } // TODO: приходит мусор, падение 04.02
            }
        };

        _filter_graph = {
            ::avfilter_graph_alloc()
            , [](auto* graph) { ::avfilter_graph_free(&graph); }
        };
        if (!outputs || !inputs || !_filter_graph) {
            throw FFmpegException { "avfilter_graph_alloc failed", AVERROR(ENOMEM) };
        }

        { /* Костыль на количество потоков */ // TODO словарик 12.02
            _filter_graph->nb_threads = 1;
        }

        initBufferSource();
        initBufferSink();

        /*
         * Set the endpoints for the filter graph. The filter_graph will
         * be linked to the graph described by filters_descr.
         */

        /*
         * The buffer source output must be connected to the input pad of
         * the first filter described by filters_descr; since the first
         * filter input label is not specified, it is set to "in" by
         * default.
         */
        outputs->name       = ::av_strdup("in");
        outputs->filter_ctx = _buffersrc_ctx.get();
        outputs->pad_idx    = 0;
        outputs->next       = nullptr;

        /*
         * The buffer sink input must be connected to the output pad of
         * the last filter described by filters_descr; since the last
         * filter output label is not specified, it is set to "out" by
         * default.
         */
        inputs->name       = ::av_strdup("out");
        inputs->filter_ctx = _buffersink_ctx.get();
        inputs->pad_idx    = 0;
        inputs->next       = nullptr;

        auto raw_inputs  { inputs.get()  };
        auto raw_outputs { outputs.get() };

        if (const auto ret {
                ::avfilter_graph_parse_ptr(
                    _filter_graph.get()
                    , _filters_descr.c_str()
                    , &raw_inputs
                    , &raw_outputs
                    , nullptr       /* context used for logging */
            )}; ret < 0) {
            throw FFmpegException { "avfilter_graph_parse_ptr failed", ret };
        }

        log_info("Filter description: " << _filters_descr);

        if (const auto ret {
                ::avfilter_graph_config(
                    _filter_graph.get()
                    , nullptr   /* context used for logging */
                )
            }; ret < 0) {
            throw FFmpegException { "avfilter_graph_config failed", ret };
        }
    }

} // namespace fpp
