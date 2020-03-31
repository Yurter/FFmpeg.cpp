#pragma once
#include <fpp/base/CodecContext.hpp>
#include <fpp/base/Frame.hpp>
#include <fpp/base/Packet.hpp>

namespace fpp {

    class EncoderContext : public CodecContext {

    public:

        EncoderContext(const SharedParameters params, Options options = {});

        PacketList          encode(const Frame& frame);
        PacketList          flush();

    private:

        void                sendFrame(const Frame& frame);
        void                sendFlushFrame();
        PacketList          receivePackets(AVRational time_base);

    };

} // namespace fpp
