#include "DecoderContext.hpp"
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

namespace fpp {

    DecoderContext::DecoderContext(const SharedParameters params, Options options)
        : CodecContext(params) {
        setName("DecCtx");
        if (!params->isDecoder()) {
            throw std::runtime_error {
                "Decoder cannot be initialized with encoder parameters"
            };
        }
        init(options);
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
        auto ret { 0 };
        while (ret == 0) {
            Frame output_frame { params->type() };
            ret = ::avcodec_receive_frame(raw(), output_frame.ptr());
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                /* Не ошибка */
                break;
            if (ret < 0) {
                throw FFmpegException {
                    utils::receive_frame_error_to_string(ret), ret
                };
            }
            output_frame.setTimeBase(params->timeBase());
            decoded_frames.push_back(output_frame);
        }
        return decoded_frames;
    }

} // namespace fpp
