#pragma once
#include <fpp/base/CodecContext.hpp>
#include <fpp/base/Frame.hpp>
#include <fpp/base/Packet.hpp>

namespace fpp {

    class EncoderContext : public CodecContext {

    public:

        EncoderContext(const SharedParameters parameters, AVRational source_time_base, Dictionary&& dictionary = Dictionary {});

        PacketList          encode(const Frame& frame);
        PacketList          flush();
        virtual void        onOpen()    override;

    private:

        void                sendFrame(const Frame& frame);
        void                sendFlushFrame();
        PacketList          receivePackets();

    private:

        AVRational          _source_time_base;

    };

} // namespace fpp
