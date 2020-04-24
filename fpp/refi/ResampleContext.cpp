#include "ResampleContext.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

namespace fpp {

    ResampleContext::ResampleContext(IOParams parameters)
        : params { parameters }
        , _samples_count { 0 }
        , _source_pts { 0 } {
        setName("Resampler");
        init();
    }

    FrameList ResampleContext::resample(const Frame source_frame) {
        sendFrame(source_frame);
        return receiveFrames();
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
                nullptr   /* existing Swr context   */
                , int64_t(out_param->channelLayout())
                , out_param->sampleFormat()
                , int(out_param->sampleRate())
                , int64_t(in_param->channelLayout())
                , in_param->sampleFormat()
                , int(in_param->sampleRate())
                , 0       /* logging level offset   */
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
        if (const auto ret { ::av_frame_get_buffer(frame.ptr(), 32) }; ret < 0) {
            throw FFmpegException { "av_frame_get_buffer failed", ret };
        }
        return frame;
    }

    void ResampleContext::sendFrame(const Frame source_frame) {
        if (const auto ret {
                ::swr_convert_frame(
                    raw()                   /* swr    */
                    , nullptr               /* output */
                    , source_frame.ptr()    /* input  */
                )
            }; ret != 0) {
            throw FFmpegException {
                "swr_convert_frame failed: "
                    + utils::swr_convert_frame_error_to_string(ret)
                , ret
            };
        }
        _source_pts = source_frame.pts();
    }

    FrameList ResampleContext::receiveFrames() {
        const auto out_param {
            std::static_pointer_cast<const AudioParameters>(params.out)
        };

        FrameList resampled_frames;
        while (::swr_get_out_samples(raw(), 0) >= out_param->frameSize()) {
            Frame output_frame { createFrame() };
            if (const auto ret {
                ::swr_convert_frame(
                    raw()                   /* swr    */
                    , output_frame.ptr()    /* output */
                    , nullptr               /* input  */
                )
            }; ret < 0) {
                throw FFmpegException {
                    "swr_convert_frame failed: "
                        + utils::swr_convert_frame_error_to_string(ret)
                    , ret
                };
            }
            stampFrame(output_frame);
            output_frame.setTimeBase(params.in->timeBase());
            resampled_frames.push_back(output_frame);
        }
        return resampled_frames;
    }

    void ResampleContext::stampFrame(Frame& output_frame) {
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
            output_frame.setPts(out_pts);
        } else {
            output_frame.setPts(NOPTS_VALUE);
        }
        _samples_count += output_frame.nbSamples();
    }

} // namespace fpp
