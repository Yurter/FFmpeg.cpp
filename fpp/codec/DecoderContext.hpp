#pragma once
#include <fpp/base/CodecContext.hpp>
#include <fpp/base/Frame.hpp>
#include <fpp/base/Packet.hpp>

namespace fpp {

    class DecoderContext : public CodecContext {

    public:

        DecoderContext(const SpParameters params, Options options = {});

        FrameVector         decode(const Packet& packet);
        FrameVector         flush();

    private:

        void                sendPacket(const Packet& packet);
        void                sendFlushPacket();
        FrameVector         receiveFrames();

    };

} // namespace fpp
