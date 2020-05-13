#include "BitStreamFilterContext.hpp"
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace fpp {

    BitStreamFilterContext::BitStreamFilterContext(const fpp::SpParameters param, const std::string_view filter_name) {
        setName("BSFContext");
        reset(
            [&]() {
                const auto bfs {
                    ::av_bsf_get_by_name(filter_name.data())
                };
                if (!bfs) {
                    throw FFmpegException {
                        "No such bitstream filter exists: "
                        + std::string { filter_name }
                    };
                }

                AVBSFContext* bfs_ctx { nullptr };
                ffmpeg_api_strict(::av_bsf_alloc, bfs, &bfs_ctx);

                param->initCodecpar(bfs_ctx->par_in);
                bfs_ctx->time_base_in = param->timeBase();

                ffmpeg_api_strict(::av_bsf_init, bfs_ctx); // TODO: memory leak on failure (28.04)
                return bfs_ctx;
            }()
            , [](AVBSFContext* ctx) {
                ::av_bsf_flush(ctx);
                ::av_bsf_free(&ctx);
            }
        );
    }

    Packet BitStreamFilterContext::filter(Packet packet) {
        Packet filtered_packet { packet.type() };
        filtered_packet.setTimeBase(packet.timeBase());
        ffmpeg_api_strict(::av_bsf_send_packet,    raw(), packet.ptr()         );
        ffmpeg_api_strict(::av_bsf_receive_packet, raw(), filtered_packet.ptr());
        return filtered_packet;
    }

} // namespace fpp
