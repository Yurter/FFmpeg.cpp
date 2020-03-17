#include "ResampleContext.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

namespace fpp {

    ResampleContext::ResampleContext(IOParams parameters)
        : params { parameters } {
        setName("Resampler");
        init();
    }

    // https://github.com/FFmpeg/FFmpeg/blob/a0ac49e38ee1d1011c394d7be67d0f08b2281526/libavfilter/af_aresample.c#L209
    FrameList ResampleContext::resample(const Frame source_frame) {
        if (const auto ret {
                ::swr_convert_frame(raw(), nullptr, source_frame.ptr())
            }; ret != 0) {
            throw FFmpegException {
                "swr_convert_frame failed: "
                    + utils::swr_convert_frame_error_to_string(ret)
                , ret
            };
        }
        const auto in_param {
            std::static_pointer_cast<const AudioParameters>(params.in)
        };
        const auto out_param {
            std::static_pointer_cast<const AudioParameters>(params.out)
        };
        FrameList resampled_frames;
        while (::swr_get_out_samples(raw(), 0) >= out_param->frameSize()) { // TODO сравнить AVERROR(EAGAIN) и swr_get_out_samples,
            Frame output_frame { createFrame() };
            if (source_frame.pts() != AV_NOPTS_VALUE) {
                const auto in_pts {
                    ::av_rescale(
                        source_frame.pts()
                        , in_param->timeBase().num * out_param->sampleRate() * in_param->sampleRate()
                        , in_param->timeBase().den
                    )
                };
//                const auto out_pts { ::swr_next_pts(raw(), INT64_MIN) };
//                const auto out_pts { ::swr_next_pts(raw(), in_pts) };
                static auto samples_count { 0 };
                const auto out_pts { ::av_rescale_q(samples_count, AVRational {1, 44'100}, AVRational { 1, 16000 }) };
//                output_frame.setPts(ROUNDED_DIV(out_pts, in_param->sampleRate()));
                output_frame.setPts(out_pts);
                samples_count += output_frame.nbSamples();
                log_error(out_pts << " " << output_frame.pts());
            } else {
                output_frame.setPts(AV_NOPTS_VALUE);
            }
            const auto ret { ::swr_convert_frame(raw(), output_frame.ptr(), nullptr) };
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                /* не ошибка */
                break;
            if (ret < 0) {
                throw FFmpegException {
                    "swr_convert_frame failed: "
                        + utils::swr_convert_frame_error_to_string(ret)
                    , ret
                };
            }
            output_frame.setTimeBase(source_frame.timeBase());
            resampled_frames.push_back(output_frame);
        }
        return resampled_frames;
    }

    // TODO https://stackoverflow.com/questions/12831761/how-to-resize-a-picture-using-ffmpegs-sws-scale
    void ResampleContext::init() {
        const auto in_param {
            std::static_pointer_cast<const AudioParameters>(params.in)
        };
        const auto out_param {
            std::static_pointer_cast<const AudioParameters>(params.out)
        };

        reset(std::shared_ptr<SwrContext> {
            ::swr_alloc_set_opts(
                nullptr   /* existing Swr context   */
                , out_param->channelLayout()
                , out_param->sampleFormat()
                , int(out_param->sampleRate())
                , in_param->channelLayout()
                , in_param->sampleFormat()
                , int(in_param->sampleRate())
                , 0       /* logging level offset   */
                , nullptr /* parent logging context */
            )
            , [](auto* ctx) { ::swr_free(&ctx); }
        });

        if (const auto ret { ::swr_init(raw()) }; ret < 0) {
            throw FFmpegException { "swr_init failed", ret };
        }

        log_info("Inited "
            << "from ["
                << "ch_layout " << utils::channel_layout_to_string(in_param->channels(), in_param->channelLayout())
                << ", smp_rate " << in_param->sampleRate()
                << ", " << in_param->sampleFormat()
                << ", nb_smp " << in_param->frameSize()
                << "] "
            << "to ["
                 << "ch_layout " << utils::channel_layout_to_string(out_param->channels(), out_param->channelLayout())
                 << ", smp_rate " << out_param->sampleRate()
                 << ", " << out_param->sampleFormat()
                 << ", nb_smp " << out_param->frameSize()
                 << "] "
        );
    }

    Frame ResampleContext::createFrame() const {
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
        if (const auto ret { ::av_frame_get_buffer(frame.ptr(), 32) }; ret < 0) {
            throw FFmpegException { "av_frame_get_buffer failed", ret };
        }
        return frame;
    }

} // namespace fpp
