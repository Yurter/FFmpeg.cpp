#include "ResampleContext.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libswresample/swresample.h>
}

namespace fpp {

    ResampleContext::ResampleContext(InOutParams parameters)
        : params { parameters }
        , _samples_count { 0 }
        , _source_pts { 0 } {
        setName("Resampler");
        init();
    }

    FrameVector ResampleContext::resample(const Frame& frame) {
        sendFrame(frame);
        return receiveFrames(frame.timeBase(), frame.streamIndex());
    }

    void ResampleContext::init() {
        const auto in_param {
            std::static_pointer_cast<const AudioParameters>(params.in)
        };
        const auto out_param {
            std::static_pointer_cast<const AudioParameters>(params.out)
        };

        reset(std::shared_ptr<SwrContext> {
            ::swr_alloc_set_opts(
                nullptr   /* existing Swr context */
                , int64_t(out_param->channelLayout())
                , out_param->sampleFormat()
                , out_param->sampleRate()
                , int64_t(in_param->channelLayout())
                , in_param->sampleFormat()
                , in_param->sampleRate()
                , 0       /* logging level offset */
                , nullptr /* parent logging context */
            )
            , [](auto* ctx) { ::swr_free(&ctx); }
        });

        ffmpeg_api_strict(swr_init, raw());

        log_info("Inited "
            , "from ["
                , "ch_layout " , utils::channel_layout_to_string(
                                        int(in_param->channels())
                                        , in_param->channelLayout()
                                    )
                , ", smp_rate ", in_param->sampleRate()
                , ", ", in_param->sampleFormat()
                , ", nb_smp ", in_param->frameSize()
                , "] "
            , "to ["
                 , "ch_layout ", utils::channel_layout_to_string(
                                        int(out_param->channels())
                                        , out_param->channelLayout()
                                    )
                 , ", smp_rate ", out_param->sampleRate()
                 , ", ", out_param->sampleFormat()
                 , ", nb_smp ", out_param->frameSize()
                 , "] "
        );
    }

    Frame ResampleContext::createFrame() const {
        Frame frame { params.out->type() };
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

    void ResampleContext::sendFrame(const Frame& frame) {
        if (const auto ret {
                ::swr_convert_frame(
                    raw()         /* swr    */
                    , nullptr     /* output */
                    , frame.ptr() /* input  */
                )
            }; ret != 0) {
            throw FFmpegException {
                "swr_convert_frame failed: "
                    + utils::swr_convert_frame_error_to_string(ret)
                , ret
            };
        }
        _source_pts = frame.pts();
    }

    FrameVector ResampleContext::receiveFrames(AVRational time_base, int stream_index) {
        const auto out_param {
            std::static_pointer_cast<const AudioParameters>(params.out)
        };

        FrameVector resampled_frames;
        while (::swr_get_out_samples(raw(), 0) >= out_param->frameSize()) {
            Frame frame { createFrame() };
            if (const auto ret {
                ::swr_convert_frame(
                    raw()         /* swr    */
                    , frame.ptr() /* output */
                    , nullptr     /* input  */
                )
            }; ret < 0) {
                throw FFmpegException {
                    "swr_convert_frame failed: "
                        + utils::swr_convert_frame_error_to_string(ret)
                    , ret
                };
            }
            stampFrame(frame);
            frame.setTimeBase(time_base);
            frame.setStreamIndex(stream_index);
            resampled_frames.push_back(frame);
        }
        return resampled_frames;
    }

    void ResampleContext::stampFrame(Frame& frame) {
        if (_source_pts != NOPTS_VALUE) {
            const auto in_param {
                std::static_pointer_cast<const AudioParameters>(params.in)
            };
            const auto out_param {
                std::static_pointer_cast<const AudioParameters>(params.out)
            };

            const auto out_pts {
                ::av_rescale_q(
                    _samples_count
                    , ::av_make_q(1, int(out_param->sampleRate()))
                    , in_param->timeBase()
                )
            };
            frame.setPts(out_pts);
        } else {
            frame.setPts(NOPTS_VALUE);
        }
        _samples_count += frame.nbSamples();
    }

} // namespace fpp
