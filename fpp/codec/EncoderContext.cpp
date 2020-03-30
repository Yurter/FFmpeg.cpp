#include "EncoderContext.hpp"
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Utils.hpp>

namespace fpp {

    EncoderContext::EncoderContext(const SharedParameters params, Options options)
        : CodecContext(params) {
        setName("EncCtx");
        if (!params->isEncoder()) {
            throw std::runtime_error {
                "Encoder cannot be initialized with decoder parameters"
            };
        }
        init(options);
    }

    PacketList EncoderContext::encode(const Frame& frame) {
        sendFrame(frame);
        return receivePackets();
    }

    PacketList EncoderContext::flush() {
        log_debug("Flushing");
        sendFlushFrame();
        return receivePackets();
    }

    void EncoderContext::sendFrame(const Frame& frame) {
        if (const auto ret {
                ::avcodec_send_frame(raw(), &frame.raw())
            }; ret != 0) {
            throw FFmpegException {
                utils::send_frame_error_to_string(ret)
                , ret
            };
        }
    }

    void EncoderContext::sendFlushFrame() {
        if (const auto ret {
                ::avcodec_send_frame(raw(), nullptr)
            }; ret != 0) {
            throw FFmpegException {
                utils::send_frame_error_to_string(ret)
                , ret
            };
        }
    }

    PacketList EncoderContext::receivePackets() {
        PacketList encoded_packets;
        auto ret { 0 };
        while (0 == ret) {
            Packet output_packet { params->type() };
            ret = ::avcodec_receive_packet(raw(), output_packet.ptr());
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                throw FFmpegException {
                    utils::receive_packet_error_to_string(ret)
                    , ret
                };
            }
            output_packet.setStreamIndex(params->streamIndex());
            output_packet.setTimeBase(raw()->time_base);
            encoded_packets.push_back(output_packet);
        }
        return encoded_packets;
    }

} // namespace fpp
