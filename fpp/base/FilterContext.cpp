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

    FilterContext::FilterContext(SpParameters parameters, const std::string& filters_descr)
        : params(parameters)
        , _filters_descr(filters_descr) {
        setName("FilterContext");
    }

    FrameVector FilterContext::filter(Frame source_frame) {
        ffmpeg_api_strict(av_buffersrc_add_frame_flags
           , _buffersrc_ctx.get()
           , source_frame.ptr()
           , AV_BUFFERSRC_FLAG_KEEP_REF
        );

        FrameVector filtered_frames;
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

        initFilterGraph();

        initBufferSource();
        initBufferSink();

        /*
         * Set the endpoints for the filter graph. The filter_graph will
         * be linked to the graph described by filters_descr.
         */
        initInputs();
        initOutputs();

        auto raw_inputs  { _inputs.get()  };
        auto raw_outputs { _outputs.get() };

        ffmpeg_api_strict(avfilter_graph_parse_ptr
            , _filter_graph.get()
            , _filters_descr.c_str()
            , &raw_inputs
            , &raw_outputs
            , nullptr /* context used for logging */
        );

        log_info("Filter description: ", _filters_descr);

        ffmpeg_api_strict(avfilter_graph_config
           , _filter_graph.get()
           , nullptr /* context used for logging */
        );

    }

    void FilterContext::initInputs() {

        _inputs = std::shared_ptr<AVFilterInOut> {
            ::avfilter_inout_alloc()
            , [](auto* filter) { /*::avfilter_inout_free(&filter);*/ } // TODO: приходит мусор, падение 04.02
        };

        if (!_inputs) {
            throw FFmpegException {
                std::string { __FUNCTION__ } + " failed"
                , AVERROR(ENOMEM)
            };
        }

        /*
         * The buffer sink input must be connected to the output pad of
         * the last filter described by filters_descr; since the last
         * filter output label is not specified, it is set to "out" by
         * default.
         */
        _inputs->name       = ::av_strdup("out");
        _inputs->filter_ctx = _buffersink_ctx.get();
        _inputs->pad_idx    = 0;
        _inputs->next       = nullptr;

    }

    void FilterContext::initOutputs() {

        _outputs = std::shared_ptr<AVFilterInOut> {
            ::avfilter_inout_alloc()
            , [](auto* filter) { /*::avfilter_inout_free(&filter);*/ } // TODO: приходит мусор, падение 04.02
        };

        if (!_outputs) {
            throw FFmpegException {
                std::string { __FUNCTION__ } + " failed"
                , AVERROR(ENOMEM)
            };
        }

        /*
         * The buffer source output must be connected to the input pad of
         * the first filter described by filters_descr; since the first
         * filter input label is not specified, it is set to "in" by
         * default.
         */
        _outputs->name       = ::av_strdup("in");
        _outputs->filter_ctx = _buffersrc_ctx.get();
        _outputs->pad_idx    = 0;
        _outputs->next       = nullptr;

    }

    void FilterContext::initFilterGraph() {
        _filter_graph = {
            ::avfilter_graph_alloc()
            , [](auto* graph) { ::avfilter_graph_free(&graph); }
        };
        if (!_filter_graph) {
            throw FFmpegException {
                "avfilter_graph_alloc failed"
                , AVERROR(ENOMEM)
            };
        }

        { /* Костыль на количество потоков */ // TODO словарик 12.02
            _filter_graph->nb_threads = 1;
        }
    }

} // namespace fpp
