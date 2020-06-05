#pragma once
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <fpp/base/MediaData.hpp>
#include <vector>

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace fpp {

    class Packet : public FFmpegObject<AVPacket>, public MediaData {

    public:

        explicit Packet(MediaType type);
        Packet(const Packet& other);
        Packet(const AVPacket& avpacket, AVRational time_base, MediaType type);
        ~Packet() override;

        Packet& operator=(const Packet& other);

        void                setPts(std::int64_t pts);
        void                setDts(std::int64_t dts);
        void                setPos(std::int64_t pos);
        void                setDuration(std::int64_t duration);
        void                setTimeBase(AVRational time_base);
        void                setStreamIndex(int stream_index);

        std::int64_t        pts()           const;
        std::int64_t        dts()           const;
        std::int64_t        duration()      const;
        AVRational          timeBase()      const;
        std::int64_t        pos()           const;
        int                 streamIndex()   const;
        bool                keyFrame()      const;

        int                 size()          const;
        std::string         toString()      const override;

    private:

        void                ref(const Packet& other);
        void                ref(const AVPacket& other, AVRational time_base);
        void                unref();

    private:

        AVRational          _time_base;

    };

    using PacketVector = std::vector<Packet>;

} // namespace fpp
