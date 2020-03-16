#pragma once
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/stream/AudioParameters.hpp>
#include <fpp/core/time/Chronometer.hpp>
#include <fpp/base/Packet.hpp>
#include <vector>

#define FROM_START  0
#define TO_END      LONG_MAX

namespace fpp {

    /* ? */ //TODO заменить вируальным мтоедодом штампа 03.02
    enum class StampType : uint8_t {
        /* Штампы сорса */
        Copy,
        Realtime,
        Offset,
        /* Штампы синка */
        Rescale,
    };

    class Stream;
    using SharedStream = std::shared_ptr<Stream>;
    using StreamVector = std::vector<SharedStream>;

                            // TODO заменить на шаред оббъект без деструктора 13.02
    class Stream : public FFmpegObject</*const*/ AVStream*>, public MediaData {

    public: // TODO сделать конструкторы приватными

        Stream(SharedParameters parameters, AVStream* avstream);
        Stream(AVStream* avstream);   // Создание реального потока
        Stream(AVStream* avstream, SharedParameters parameters);      //TODO сделать приватным 23.01 (используется в OutputContext)
        Stream(SharedParameters params);    // Создание виртуального потока //TODO не используется (см 1й конструктор) 24.01
        Stream(const Stream& other) = delete;
        virtual ~Stream() override = default;

//        void                init();
        virtual std::string toString() const override final;

        void                stampPacket(Packet& packet);
        bool                timeIsOver() const;

        void                setUsed(bool value);
        void                setStampType(StampType value);
        void                setStartTimePoint(int64_t value);
        void                setEndTimePoint(int64_t value);

        int64_t             index()             const;
        bool                used()              const;
        StampType           stampType()         const;
        int64_t             startTimePoint()    const;
        int64_t             endTimePoint()      const;
        int64_t             packetIndex()       const;

        AVCodecParameters*  codecParams();

    private:

        void                initCodecpar();

    public:

        SharedParameters    params;

    private:

        bool                _used;
        StampType           _stamp_type;
        Chronometer         _chronometer;

        int64_t             _prev_dts;
        int64_t             _prev_pts;
        int64_t             _packet_index;

        int64_t             _packet_dts_delta;
        int64_t             _packet_pts_delta;
        int64_t             _packet_duration;

        int64_t             _pts_offset;
        int64_t             _dts_offset;

        int64_t             _start_time_point;
        int64_t             _end_time_point;

    };

    inline SharedStream make_input_stream(AVStream* avstream) {
        return std::make_shared<Stream>(avstream);
    }

    inline SharedStream make_output_stream(AVStream* avstream, const SharedParameters params) {
        return std::make_shared<Stream>(params, avstream);
    }

    inline SharedStream make_virtual_input_stream(const SharedParameters params) {
        return std::make_shared<Stream>(nullptr, params);
    }

    inline SharedStream make_virtual_output_stream(const SharedParameters params) {
        return std::make_shared<Stream>(nullptr, params);
    }

} // namespace fpp
