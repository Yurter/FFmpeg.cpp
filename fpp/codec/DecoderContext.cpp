#include "DecoderContext.hpp"
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

namespace fpp {

    DecoderContext::DecoderContext(const SharedParameters parameters, const AVStream* test_stream, Dictionary&& dictionary)
        : CodecContext(parameters, test_stream) {
        setName("DecCtx");
        if (!parameters->isDecoder()) {
            throw std::runtime_error {
                "Decoder cannot be initialized with encoder parameters"
            };
        }
        init(std::move(dictionary));
    }

    FrameList DecoderContext::decode(const Packet& packet) {
        sendPacket(packet);
        return receiveFrames();
    }

    FrameList DecoderContext::flush() {
        log_debug("Flushing");
        sendFlushPacket();
        return receiveFrames();
    }

    void DecoderContext::sendPacket(const Packet& packet) {
        if (const auto ret {
                ::avcodec_send_packet(raw(), &packet.raw())
            }; ret != 0) {
            throw FFmpegException { utils::send_packet_error_to_string(ret), ret };
        }
    }

    void DecoderContext::sendFlushPacket() {
        if (const auto ret {
                ::avcodec_send_packet(raw(), nullptr)
            }; ret != 0) {
            throw FFmpegException { utils::send_packet_error_to_string(ret), ret };
        }
    }

    FrameList DecoderContext::receiveFrames() {
        FrameList decoded_frames;
        int ret { 0 };
        while (ret == 0) {
            Frame output_frame { params->type() };
            ret = ::avcodec_receive_frame(raw(), &output_frame.raw());
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                /* Не ошибка */
                break;
            if (ret < 0) {
                throw FFmpegException { utils::receive_frame_error_to_string(ret), ret };
            }
//            if (output_frame.isVideo()) {
//                output_frame.raw().pict_type = AV_PICTURE_TYPE_NONE;
//            }
            decoded_frames.push_back(output_frame);
        }
        return decoded_frames;
    }

} // namespace fpp
