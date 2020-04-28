#include "BitStreamFilterContext.hpp"

#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace fpp {

    BitStreamFilterContext::BitStreamFilterContext(const std::string_view filter_name) {
        setName("BSFContext");

        const AVBitStreamFilter* bfs {
            ::av_bsf_get_by_name(filter_name.data())
        };
        if (!bfs) {
            throw FFmpegException {
                "no such bitstream filter exists: " + std::string { filter_name }
            };
        }

        AVBSFContext* bfs_ctx { nullptr };

        ffmpeg_api_strict(::av_bsf_alloc, bfs, &bfs_ctx);
        ffmpeg_api_strict(::av_bsf_init, bfs_ctx);

        reset({
            nullptr, [](AVBSFContext* ctx) {
                ::av_bsf_flush(ctx);
                ::av_bsf_free(&ctx);
            }
        });
    }

    Packet BitStreamFilterContext::filter(Packet packet) {
        Packet filtered_packet { packet.type() };
        ffmpeg_api_strict(::av_bsf_send_packet,    raw(), packet.ptr()         );
        ffmpeg_api_strict(::av_bsf_receive_packet, raw(), filtered_packet.ptr());
        return filtered_packet;
    }

} // namespace fpp
