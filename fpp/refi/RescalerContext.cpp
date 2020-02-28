#include "RescalerContext.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

namespace fpp {

    RescalerContext::RescalerContext(IOParams parameters)
        : params { parameters } {
        init();
    }

    Frame RescalerContext::scale(Frame source_frame) {
        Frame rescaled_frame { createFrame() };
        ::sws_scale(raw(), source_frame.raw().data
            , source_frame.raw().linesize, 0, source_frame.raw().height
            , rescaled_frame.raw().data, rescaled_frame.raw().linesize
        );
        ::av_frame_copy_props(rescaled_frame.ptr(), source_frame.ptr());
        return rescaled_frame;
    }

    void RescalerContext::init() {
        const auto in_param {
            std::dynamic_pointer_cast<const VideoParameters>(params.in)
        };
        const auto out_param {
            std::dynamic_pointer_cast<const VideoParameters>(params.out)
        };

        reset(std::shared_ptr<SwsContext> {
            ::sws_getContext(
                int(in_param->width()), int(in_param->height()), in_param->pixelFormat()
                , int(out_param->width()), int(out_param->height()), out_param->pixelFormat()
                , SWS_BICUBIC, nullptr, nullptr, nullptr
            )
            , [](auto* ctx) { ::sws_freeContext(ctx); }
        });

        log_info("Inited "
            << "from "
                << "[" << in_param->width()
                << "x"  << in_param->height()
                << ", " << in_param->pixelFormat()
                << "] "
            << "to "
                << "[" << out_param->width()
                << "x"  << out_param->height()
                << ", " << out_param->pixelFormat()
                << "]"
        );
    }

    Frame RescalerContext::createFrame() const {
        Frame frame { params.out->type() };
        const auto output_params {
            std::static_pointer_cast<const VideoParameters>(params.out)
        };
        frame.raw().format = output_params->pixelFormat();
        frame.raw().width  = int(output_params->width());
        frame.raw().height = int(output_params->height());
        if (const auto ret { ::av_frame_get_buffer(frame.ptr(), 32) }; ret < 0) {
            throw FFmpegException { "av_frame_get_buffer failed", ret };
        }
        return frame;
    }

} // namespace fpp
