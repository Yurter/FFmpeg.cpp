#pragma once
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>
#include <list>

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace fpp {

    class Packet : public FFmpegObject<AVPacket>, public MediaData {

    public:

        Packet(MediaType type);
        Packet(const Packet& other);
        Packet(const AVPacket& avpacket, AVRational time_base, MediaType type);
        virtual ~Packet() override;

        Packet& operator=(const Packet& other);

        void                setPts(int64_t pts);
        void                setDts(int64_t dts);
        void                setPos(int64_t pos);
        void                setDuration(int64_t duration);
        void                setTimeBase(AVRational time_base);
        void                setStreamIndex(int64_t stream_index);

        int64_t             pts()           const;
        int64_t             dts()           const;
        int64_t             duration()      const;
        AVRational          timeBase()      const;
        int64_t             pos()           const;
        int64_t             streamIndex()   const;
        bool                keyFrame()      const;

        size_t              size()          const;
        virtual std::string toString()      const override;

    private:

        void                ref(const Packet& other);
        void                ref(const AVPacket& other, AVRational time_base);
        void                unref();

    private:

        AVRational          _time_base;

    };

    using PacketList = std::list<Packet>;

} // namespace fpp
