#pragma once
#include <fpp/base/CodecContext.hpp>
#include <fpp/base/Frame.hpp>
#include <fpp/base/Packet.hpp>

namespace fpp {

    class DecoderContext : public CodecContext {

    public:

        DecoderContext(const SharedParameters parameters, const AVStream* test_stream, Dictionary&& dictionary = Dictionary {});

        FrameList           decode(const Packet& packet);
        FrameList           flush();

    private:

        void                sendPacket(const Packet& packet);
        void                sendFlushPacket();
        FrameList           receiveFrames();

    };

} // namespace fpp
