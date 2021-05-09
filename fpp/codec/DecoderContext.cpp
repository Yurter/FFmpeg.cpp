#include "DecoderContext.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <cassert>

namespace fpp {

    DecoderContext::DecoderContext(const SpParameters params, Options options)
        : CodecContext(params) {
        assert(params->isDecoder());
        init(options);
    }

    FrameVector DecoderContext::decode(const Packet& packet) {
        sendPacket(packet);
        return receiveFrames(packet.timeBase(), packet.streamIndex());
    }

    FrameVector DecoderContext::flush(AVRational time_base, int stream_index) {
        sendFlushPacket();
        return receiveFrames(time_base, stream_index);
    }

    void DecoderContext::sendPacket(const Packet& packet) {
        if (const auto ret {
                ::avcodec_send_packet(raw(), &packet.raw())
            }; ret != 0) {
            log_error() << utils::send_packet_error_to_string(ret);
        }
    }

    void DecoderContext::sendFlushPacket() {
        if (const auto ret {
                ::avcodec_send_packet(raw(), nullptr)
            }; ret != 0) {
            throw FFmpegException {
                utils::send_packet_error_to_string(ret)
            };
        }
    }

    FrameVector DecoderContext::receiveFrames(AVRational time_base, int stream_index) {
        FrameVector decoded_frames;
        auto ret { 0 };
        while (ret == 0) {
            Frame output_frame { params->type() };
            ret = ::avcodec_receive_frame(raw(), output_frame.ptr());
            if ((ERROR_AGAIN == ret) || (ERROR_EOF == ret)) {
                break; /* not an error - just an exit code */
            }
            if (ret < 0) {
                throw FFmpegException {
                    utils::receive_frame_error_to_string(ret)
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
