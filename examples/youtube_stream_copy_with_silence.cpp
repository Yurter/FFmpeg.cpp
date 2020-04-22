#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/ResampleContext.hpp>
#include <fpp/core/Utils.hpp>
#include "examples.hpp"

void youtube_stream_copy_with_silence() {

    /* create video source */
    fpp::InputFormatContext source {
        "rtsp://205.120.142.79/live/ch00_0"
    };

//    fpp::Options video_options {
//        { "use_wallclock_as_timestamps", "1" }
//    };

    /* open video source */
    if (!source.open(/*video_options*/)) {
        return;
    }

    /* check input video stream */
    if (!source.stream(fpp::MediaType::Video)) {
        std::cout << "Youtube require video stream\n";
        return;
    }

    /* create audio source */
    fpp::InputFormatContext silence {
        fpp::InputFormatContext::silence(44'100)
    };

//    fpp::Options audio_options {
//        { "use_wallclock_as_timestamps", "1" }
//    };

    /* open audio source */
    if (!silence.open()) {
        return;
    }

    /* create sink */
    const std::string stream_key {
        "5xd3-pf7r-r9z8-9pjx"
    };
    fpp::OutputFormatContext sink {
        "rtmp://a.rtmp.youtube.com/live2/"
        + stream_key
    };

    /* copy source's video stream to sink */
    sink.copyStream(source.stream(fpp::MediaType::Video));

    /* ? */
    const auto in_audio_params {
        silence.stream(fpp::MediaType::Audio)->params
    };
    const auto out_audio_params { fpp::utils::make_youtube_audio_params() };
    out_audio_params->completeFrom(in_audio_params);
    sink.createStream(out_audio_params);

    /* open sink */
    if (!sink.open()) {
        return;
    }

    auto last_video_dts { 0ll };
    auto last_audio_dts { 0ll };

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_packet {
        [&]() {
            input_packet = source.read();
//            if (last_video_dts > last_audio_dts) {
//                input_packet = silence.read();
//                last_audio_dts = ::av_rescale_q(input_packet.dts(), input_packet.timeBase(), DEFAULT_TIME_BASE);
//            }
//            else {
//                input_packet = source.read();
//                last_video_dts = ::av_rescale_q(input_packet.dts(), input_packet.timeBase(), DEFAULT_TIME_BASE);
//            }
            return !input_packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        sink.write(input_packet);
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
