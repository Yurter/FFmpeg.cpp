#include "RescaleContext.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

namespace fpp {

    RescaleContext::RescaleContext(IOParams parameters)
        : params { parameters } {
        setName("Rescaler");
        init();
    }

    Frame RescaleContext::scale(const Frame source_frame) {
        Frame rescaled_frame { createFrame() };
        ::sws_scale(
            raw()
            , source_frame.raw().data       /* srcSlice[]  */
            , source_frame.raw().linesize   /* srcStride[] */
            , 0                             /* srcSliceY   */
            , source_frame.raw().height     /* srcSliceH   */
            , rescaled_frame.raw().data     /* dst[]       */
            , rescaled_frame.raw().linesize /* dstStride[] */
        );
        ::av_frame_copy_props(rescaled_frame.ptr(), source_frame.ptr());
        rescaled_frame.setTimeBase(params.in->timeBase());
        return rescaled_frame;
    }

    void RescaleContext::init() {
        const auto in_param {
            std::static_pointer_cast<const VideoParameters>(params.in)
        };
        const auto out_param {
            std::static_pointer_cast<const VideoParameters>(params.out)
        };

        reset(std::shared_ptr<SwsContext> {
            ::sws_getContext(
                int(in_param->width()), int(in_param->height()), in_param->pixelFormat()
                , int(out_param->width()), int(out_param->height()), out_param->pixelFormat()
                , SWS_BICUBIC /* flags     */
                , nullptr     /* srcFilter */
                , nullptr     /* dstFilter */
                , nullptr     /* param     */
            )
            , [](auto* ctx) { ::sws_freeContext(ctx); }
        });

        log_info("Inited "
            , "from "
                , "[" , in_param->width()
                , "x"  , in_param->height()
                , ", " , in_param->pixelFormat()
                , "] "
            , "to "
                , "[" , out_param->width()
                , "x"  , out_param->height()
                , ", " , out_param->pixelFormat()
                , "]"
        );
    }

    Frame RescaleContext::createFrame() const {
        Frame frame { params.out->type() };
        const auto output_params {
            std::static_pointer_cast<const VideoParameters>(params.out)
        };
        frame.raw().format = output_params->pixelFormat();
        frame.raw().width  = int(output_params->width());
        frame.raw().height = int(output_params->height());
        ffmpeg_api_strict(av_frame_get_buffer, frame.ptr(), 32);
        return frame;
    }

} // namespace fpp
