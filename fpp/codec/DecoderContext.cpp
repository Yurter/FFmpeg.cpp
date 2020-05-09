#include "DecoderContext.hpp"
#include <fpp/core/Logger.hpp>
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

namespace fpp {

    DecoderContext::DecoderContext(const SpParameters params, Options options)
        : CodecContext(params) {
        setName("DecCtx");
        if (!params->isDecoder()) {
            throw std::runtime_error {
                "Decoder cannot be initialized with encoder parameters"
            };
        }
        init(options);
    }

    FrameVector DecoderContext::decode(const Packet& packet) {
        sendPacket(packet);
        return receiveFrames(packet.timeBase(), packet.streamIndex());
    }

    FrameVector DecoderContext::flush(AVRational time_base, int64_t stream_index) {
        sendFlushPacket();
        return receiveFrames(time_base, stream_index);
    }

    void DecoderContext::sendPacket(const Packet& packet) {
        if (const auto ret {
                ::avcodec_send_packet(raw(), &packet.raw())
            }; ret != 0) {
            log_error(utils::send_packet_error_to_string(ret));
        }
    }

    void DecoderContext::sendFlushPacket() {
        if (const auto ret {
                ::avcodec_send_packet(raw(), nullptr)
            }; ret != 0) {
            throw FFmpegException { utils::send_packet_error_to_string(ret), ret };
        }
    }

    FrameVector DecoderContext::receiveFrames(AVRational time_base, int64_t stream_index) {
        FrameVector decoded_frames;
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
            output_frame.setTimeBase(time_base);
            output_frame.setStreamIndex(stream_index);
            output_frame.raw().pict_type = AV_PICTURE_TYPE_NONE; // TODO check it 0904
            decoded_frames.push_back(output_frame);
        }
        return decoded_frames;
    }

} // namespace fpp
