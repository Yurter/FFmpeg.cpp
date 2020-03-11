#include "EncoderContext.hpp"
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Utils.hpp>

#include <fpp/stream/AudioParameters.hpp> //TODO убрать, см onOpen() 02.02

namespace fpp {

    EncoderContext::EncoderContext(const SharedStream stream, Dictionary&& dictionary)
        : CodecContext(stream) {
        setName("EncCtx");
        if (!stream->params->isEncoder()) {
            throw std::runtime_error {
                "Encoder cannot be initialized with decoder parameters"
            };
        }
        init(std::move(dictionary));
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

//    void EncoderContext::onOpen() { //TODO 11.03
//        if (params->typeIs(MediaType::Audio)) {
//            static_cast<AudioParameters * const>(params.get())->setFrameSize(raw()->frame_size); //TODO не надежно: нет гарантий, что кодек откроется раньше, чем рескейлер начнет работу
//        }
//    }

    void EncoderContext::sendFrame(const Frame& frame) {
        if (const auto ret {
                ::avcodec_send_frame(raw(), &frame.raw())
            }; ret != 0) {
            throw FFmpegException { utils::send_frame_error_to_string(ret), ret };
        }
    }

    void EncoderContext::sendFlushFrame() {
        if (const auto ret {
                ::avcodec_send_frame(raw(), nullptr)
            }; ret != 0) {
            throw FFmpegException { utils::send_frame_error_to_string(ret), ret };
        }
    }

    PacketList EncoderContext::receivePackets() {
        PacketList encoded_packets;
        auto ret { 0 };
        while (0 == ret) {
            Packet output_packet { params->type() };
//            av_init_packet(output_packet.ptr());
            const auto ret { ::avcodec_receive_packet(raw(), &output_packet.raw()) };
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                throw FFmpegException { utils::receive_packet_error_to_string(ret), ret };
            }
            output_packet.setType(params->type());
            throw std::runtime_error { "TODO _source_time_base 11.03" };
//            output_packet.setTimeBase(_source_time_base);
            output_packet.setStreamIndex(params->streamIndex());
            output_packet.setDts(output_packet.pts()); //TODO костыль, разобраться, почему смещение во времени (0, -45)
            encoded_packets.push_back(output_packet);
        }
        return encoded_packets;
    }

} // namespace fpp
