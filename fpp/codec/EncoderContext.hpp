#pragma once
#include <fpp/base/CodecContext.hpp>
#include <fpp/base/Frame.hpp>
#include <fpp/base/Packet.hpp>

namespace fpp {

    class EncoderContext : public CodecContext {

    public:

        EncoderContext(const SpParameters params, Options options = {});

        PacketVector        encode(const Frame& frame);
        PacketVector        flush(AVRational time_base, int64_t stream_index);

    private:

        void                sendFrame(const Frame& frame);
        void                sendFlushFrame();
        PacketVector        receivePackets(AVRational time_base, int64_t stream_index);

    };

} // namespace fpp
