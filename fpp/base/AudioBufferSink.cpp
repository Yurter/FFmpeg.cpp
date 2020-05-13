#include "AudioBufferSink.hpp"
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavfilter/buffersink.h>
}

namespace fpp {

    AudioBufferSink::AudioBufferSink(const SpAudioParameters par
                                     , AVFilterGraph* graph
                                     , const std::string_view unique_id)
        : FilterContext(graph, "buffer", unique_id, "", nullptr)
        , _params { par } {
        setName("AudioBufferSink");
    }

    FrameVector AudioBufferSink::read() {
        FrameVector filtered_frames;
        auto ret { 0 };
        while (ret == 0) {
            Frame output_frame { MediaType::Audio };
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

    Frame AudioBufferSink::createFrame() const {
        Frame frame { MediaType::Audio };
        /* Set the frame's parameters, especially its size and format.
         * av_frame_get_buffer needs this to allocate memory for the
         * audio samples of the frame.
         * Default channel layouts based on the number of channels
         * are assumed for simplicity. */
        frame.raw().nb_samples     = _params->frameSize();
        frame.raw().channel_layout = _params->channelLayout();
        frame.raw().format         = _params->sampleFormat();
        frame.raw().sample_rate    = _params->sampleRate();
        /* Allocate the samples of the created frame. This call will make
         * sure that the audio frame can hold as many samples as specified. */
        constexpr auto align { 32 };
        ffmpeg_api_strict(av_frame_get_buffer, frame.ptr(), align);
        return frame;
    }

} // namespace fpp
