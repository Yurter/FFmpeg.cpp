#include "FilterContext.hpp"
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
}

namespace fpp {

    FilterContext::FilterContext(AVFilterGraph* graph
                                 , const std::string_view name
                                 , const std::string_view unique_id
                                 , const std::string_view args
                                 , void* opaque)
        : _nb_input_pads  { 0 }
        , _nb_output_pads { 0 } {
        setName("FilterContext");        
        reset(
            [&]() {
                AVFilterContext* flt_ctx { nullptr };
                ffmpeg_api_strict(avfilter_graph_create_filter
                    , &flt_ctx
                    , getFilterByName(name)
                    , (std::string { name } + '_' + unique_id.data()).c_str()
                    , args.data()
                    , opaque
                    , graph
                );
                return flt_ctx;
            }()
            , [](auto* ctx) { ::avfilter_free(ctx); }
        );
    }

    void FilterContext::linkTo(FilterContext& other) {
        ffmpeg_api_strict(avfilter_link
            , raw()
            , _nb_output_pads++
            , other.raw()
            , other._nb_input_pads++
        );
    }

    FrameVector FilterContext::read() {
        FrameVector filtered_frames;
        auto ret { 0 };
        while (ret == 0) {
            Frame output_frame { MediaType::Unknown };
            ret = ::av_buffersink_get_frame(raw(), output_frame.ptr());
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                throw FFmpegException { "av_buffersink_get_frame failed" };
            }
            output_frame.setTimeBase({ 1, 1 });
            filtered_frames.push_back(output_frame);
        }
        return filtered_frames;
    }

    void FilterContext::write(const Frame& frame) {
        ffmpeg_api_strict(av_buffersrc_write_frame, raw(), frame.ptr());
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

    Frame FilterContext::createFrame() const {
        Frame frame { MediaType::Audio };
        const auto out_param {
            std::static_pointer_cast<const AudioParameters>(params.out)
        };
        /* Set the frame's parameters, especially its size and format.
         * av_frame_get_buffer needs this to allocate memory for the
         * audio samples of the frame.
         * Default channel layouts based on the number of channels
         * are assumed for simplicity. */
        frame.raw().nb_samples     = int(out_param->frameSize());
        frame.raw().channel_layout = out_param->channelLayout();
        frame.raw().format         = out_param->sampleFormat();
        frame.raw().sample_rate    = int(out_param->sampleRate());
        /* Allocate the samples of the created frame. This call will make
         * sure that the audio frame can hold as many samples as specified. */
        constexpr auto align { 32 };
        ffmpeg_api_strict(av_frame_get_buffer, frame.ptr(), align);
        return frame;
    }

} // namespace fpp
