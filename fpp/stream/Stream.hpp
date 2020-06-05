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

        explicit Stream(AVStream* avstream);
        Stream(AVStream* avstream, const SpParameters parameters);

        std::string         toString() const override final;
        void                initCodecpar();

        void                stampPacket(Packet& packet);
        bool                timeIsOver() const;

        void                setIndex(int value);
        void                setDuration(std::int64_t duration);
        void                setStartTimePoint(std::int64_t msec);
        void                setEndTimePoint(std::int64_t msec);

        void                stampFromZero(bool value);

        std::int64_t        index()             const;
        std::int64_t        duration()          const;
        std::int64_t        startTimePoint()    const;
        std::int64_t        endTimePoint()      const;
        std::int64_t        packetIndex()       const;

        AVCodecParameters*  codecpar();

        void                addMetadata(const std::string_view key, const std::string_view value);

    private:

        void                increaseDuration(const std::int64_t value);

        void                shiftStamps(Packet& packet);
        void                calculatePacketDuration(Packet& packet);
//        void                avoidNegativeTimestamp(Packet& packet);
//        void                checkStampMonotonicity(Packet& packet);
//        void                checkDtsPtsOrder(Packet& packet);

    public:

        SpParameters    params;

    private:

        std::int64_t        _prev_dts;
        std::int64_t        _prev_pts;
        std::int64_t        _packet_index;

        std::int64_t        _start_time_point; // TODO not used 25.03
        std::int64_t        _end_time_point;

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
