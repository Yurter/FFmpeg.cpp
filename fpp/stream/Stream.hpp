#pragma once
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/stream/AudioParameters.hpp>
#include <fpp/core/time/Chronometer.hpp>
#include <fpp/base/Packet.hpp>
#include <vector>

#define FROM_START  0
#define TO_END      LLONG_MAX

namespace fpp {

    class Stream;
    using SharedStream = std::shared_ptr<Stream>;
    using StreamVector = std::vector<SharedStream>;

    class Stream : public FFmpegObject<AVStream*>, public MediaData {

        Stream(AVStream* avstream, MediaType type);

    public:

        Stream(AVStream* avstream);
        Stream(AVStream* avstream, const SharedParameters parameters);

        virtual ~Stream() override = default;

        virtual std::string toString() const override final;

        void                stampPacket(Packet& packet);
        bool                timeIsOver() const;

        void                setIndex(int64_t value);
        void                setStartTimePoint(int64_t msec);
        void                setEndTimePoint(int64_t msec); // TODO doesnt work in OutFmtCtx 24.03

        int64_t             index()             const;
        int64_t             startTimePoint()    const;
        int64_t             endTimePoint()      const;
        int64_t             packetIndex()       const;

        AVCodecParameters*  codecpar();

    private:

        void                initCodecpar();
        void                checkStampMonotonicity(Packet& packet);

    public:

        SharedParameters    params;

    private:

        int64_t             _prev_dts;
        int64_t             _prev_pts;
        int64_t             _packet_index;

        int64_t             _start_time_point;
        int64_t             _end_time_point;

    public:

        static inline SharedStream make_input_stream(AVStream* avstream) {
            return std::make_shared<Stream>(avstream);
        }

        static inline SharedStream make_output_stream(AVStream* avstream, const SharedParameters parameters) {
            return std::make_shared<Stream>(avstream, parameters);
        }

    };

} // namespace fpp
