#include "ResamplerContext.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

namespace fpp {

    ResamplerContext::ResamplerContext(IOParams parameters)
        : params { parameters } {
        init();
    }

    FrameList ResamplerContext::resample(Frame source_frame) {
        if (::swr_convert_frame(raw(), nullptr, source_frame.ptr()) != 0) {
            throw FFmpegException { "swr_convert_frame failed" };
        }
        const auto audio_params { std::static_pointer_cast<const AudioParameters>(params.out) };
        FrameList resampled_frames;
        while (::swr_get_out_samples(raw(), 0) >= audio_params->frameSize()) { // TODO сравнить AVERROR(EAGAIN) и swr_get_out_samples,
            Frame output_frame { createFrame() };
            const auto ret { ::swr_convert_frame(raw(), output_frame.ptr(), nullptr) };
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                /* не ошибка */
                break;
            if (ret < 0) {
                throw FFmpegException { "swr_convert_frame failed", ret };
            }
            output_frame.setPts(source_frame.pts()); //TODO проверить необходимость ручной установки 24.01
            resampled_frames.push_back(output_frame);
        }
        return resampled_frames;
    }

    // TODO https://stackoverflow.com/questions/12831761/how-to-resize-a-picture-using-ffmpegs-sws-scale
    void ResamplerContext::init() {
        const auto in_param {
            std::static_pointer_cast<const AudioParameters>(params.in)
        };
        const auto out_param {
            std::static_pointer_cast<const AudioParameters>(params.out)
        };

        reset(std::shared_ptr<SwrContext> {
            ::swr_alloc_set_opts(
                nullptr     /* existing Swr context     */
                , ::av_get_default_channel_layout(int(out_param->channels())) //TODO юзать метод setChannelLayout() 24.01
                , out_param->sampleFormat()
                , int(out_param->sampleRate())
                , ::av_get_default_channel_layout(int(in_param->channels()))  //TODO юзать метод setChannelLayout() 24.01
                , in_param->sampleFormat()
                , int(in_param->sampleRate())
                , 0         /* logging level offset     */
                , nullptr   /* parent logging context   */
            )
            , [](auto* ctx) { ::swr_free(&ctx); }
        });

        if (const auto ret { ::swr_init(raw()) }; ret < 0) {
            throw FFmpegException { "swr_init failed", ret };
        }

        log_info("Inited "
            << "from "
                << "ch_layout " << in_param->channelLayout()
                << ", sample_rate " << in_param->sampleFormat()
                << ", " << in_param->sampleFormat()
                << "] "
            << "to "
                 << "ch_layout " << out_param->channelLayout()
                 << ", sample_rate " << out_param->sampleFormat()
                 << ", " << out_param->sampleFormat()
                 << "] "
        );
    }

    Frame ResamplerContext::createFrame() const {
        Frame frame { params.out->type() };
        const auto out_param { std::static_pointer_cast<const AudioParameters>(params.out) };
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
        if (const auto ret { ::av_frame_get_buffer(frame.ptr(), 0) }; ret < 0) {
            throw FFmpegException { "av_frame_get_buffer failed", ret };
        }
        return frame;
    }

} // namespace fpp
