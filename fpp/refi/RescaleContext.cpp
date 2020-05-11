#include "RescaleContext.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libswscale/swscale.h>
}

namespace fpp {

    RescaleContext::RescaleContext(InOutParams parameters)
        : params { parameters } {
        setName("Rescaler");
        init();
    }

    Frame RescaleContext::scale(const Frame& frame) {
        Frame rescaled_frame { createFrame() };
        ::sws_scale(
            raw()
            , frame.raw().data              /* srcSlice[]  */
            , frame.raw().linesize          /* srcStride[] */
            , 0                             /* srcSliceY   */
            , frame.raw().height            /* srcSliceH   */
            , rescaled_frame.raw().data     /* dst[]       */
            , rescaled_frame.raw().linesize /* dstStride[] */
        );
        ::av_frame_copy_props(rescaled_frame.ptr(), frame.ptr());
        rescaled_frame.setTimeBase(frame.timeBase());
        rescaled_frame.setStreamIndex(frame.streamIndex());
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
                , '[' ,  in_param->width()
                , 'x'  , in_param->height()
                , ", " , in_param->pixelFormat()
                , "] "
            , "to "
                , '[' ,  out_param->width()
                , 'x'  , out_param->height()
                , ", " , out_param->pixelFormat()
                , ']'
        );
    }

    Frame RescaleContext::createFrame() const {
        Frame frame { params.out->type() };
        const auto output_params {
            std::static_pointer_cast<const VideoParameters>(params.out)
        };
        frame.raw().format = output_params->pixelFormat();
        frame.raw().width  = output_params->width();
        frame.raw().height = output_params->height();
        constexpr auto align { 32 };
        ffmpeg_api_strict(av_frame_get_buffer, frame.ptr(), align);
        return frame;
    }

} // namespace fpp
