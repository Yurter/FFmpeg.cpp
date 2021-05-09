#include "EncoderContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <cassert>

namespace fpp {

    EncoderContext::EncoderContext(const SpParameters params, Options options)
        : CodecContext(params) {
        assert(params->isEncoder());
        init(options);
    }

    PacketVector EncoderContext::encode(const Frame& frame) {
        sendFrame(frame);
        return receivePackets(frame.timeBase(), frame.streamIndex());
    }

    PacketVector EncoderContext::flush(AVRational time_base, int stream_index) {
        sendFlushFrame();
        return receivePackets(time_base, stream_index);
    }

    void EncoderContext::sendFrame(const Frame& frame) {
        if (const auto ret {
                ::avcodec_send_frame(raw(), frame.ptr())
            }; ret != 0) {
            throw FFmpegException {
                utils::send_frame_error_to_string(ret)
            };
        }
    }

    void EncoderContext::sendFlushFrame() {
        if (const auto ret {
                ::avcodec_send_frame(raw(), nullptr)
            }; ret != 0) {
            throw FFmpegException {
                utils::send_frame_error_to_string(ret)
            };
        }
    }

    PacketVector EncoderContext::receivePackets(AVRational time_base, int stream_index) {
        PacketVector encoded_packets;
        auto ret { 0 };
        while (0 == ret) {
            Packet packet { params->type() };
            ret = ::avcodec_receive_packet(raw(), packet.ptr());
            if ((ERROR_AGAIN == ret) || (ERROR_EOF == ret)) {
                break; /* not an error - just an exit code */
            }
            if (ret < 0) {
                throw FFmpegException {
                    utils::receive_packet_error_to_string(ret)
                };
            }
            packet.setStreamIndex(stream_index);
            packet.setTimeBase(time_base);
            encoded_packets.push_back(packet);
        }
        return encoded_packets;
    }

} // namespace fpp
