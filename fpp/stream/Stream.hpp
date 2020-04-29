#pragma once
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/stream/AudioParameters.hpp>
#include <fpp/base/Packet.hpp>
#include <vector>

constexpr auto FROM_START { 0ll       };
constexpr auto TO_END     { LLONG_MAX };

namespace fpp {

    class Stream;
    using SharedStream = std::shared_ptr<Stream>;
    using StreamVector = std::vector<SharedStream>;

    class Stream : public FFmpegObject<AVStream*>, public MediaData {

        Stream(AVStream* avstream, MediaType type);

    public:

        Stream(AVStream* avstream);
        Stream(AVStream* avstream, const SpParameters parameters);

        std::string         toString() const override final;

        void                stampPacket(Packet& packet);
        bool                timeIsOver() const;

        void                setIndex(int64_t value);
        void                setStartTimePoint(int64_t msec);
        void                setEndTimePoint(int64_t msec);

        void                stampFromZero(bool value);

        int64_t             index()             const;
        int64_t             startTimePoint()    const;
        int64_t             endTimePoint()      const;
        int64_t             packetIndex()       const;

        AVCodecParameters*  codecpar();

    private:

        void                shiftStamps(Packet& packet);
        void                calculatePacketDuration(Packet& packet);
//        void                avoidNegativeTimestamp(Packet& packet);
//        void                checkStampMonotonicity(Packet& packet);
//        void                checkDtsPtsOrder(Packet& packet);

    public:

        SpParameters    params;

    private:

        int64_t             _prev_dts;
        int64_t             _prev_pts;
        int64_t             _packet_index;

        int64_t             _start_time_point; // TODO not used 25.03
        int64_t             _end_time_point;

        bool                _stamp_from_zero;

    public:

        static inline SharedStream make_input_stream(AVStream* avstream) {
            return std::make_shared<Stream>(avstream);
        }

        static inline SharedStream make_output_stream(AVStream* avstream, const SpParameters parameters) {
            return std::make_shared<Stream>(avstream, parameters);
        }

    };

} // namespace fpp
