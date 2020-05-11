#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/ResampleContext.hpp>
#include <fpp/refi/RescaleContext.hpp>
#include <fpp/refi/LinearFilterGraph.hpp>

void timelapase() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://91.197.91.139/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink {
        "timelapse.flv"
    };

    const auto out_params { fpp::VideoParameters::make_shared() };
    out_params->setEncoder(AVCodecID::AV_CODEC_ID_H264);
    out_params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV420P);
    out_params->setGopSize(24);

    const auto in_params { source.stream(fpp::MediaType::Video)->params };
    out_params->completeFrom(in_params);

    /* create video stream with predefined params */
    sink.createStream(out_params);

    /* create decoder */
    fpp::DecoderContext video_decoder {
        source.stream(fpp::MediaType::Video)->params
    };

    /* create encoder's options */
    fpp::Options video_options {
          { "threads",      "1"           }
        , { "thread_type",  "slice"       }
        , { "preset",       "ultrafast"   }
        , { "crf",          "15"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };

    /* create encoders */
    fpp::EncoderContext video_encoder {
        sink.stream(fpp::MediaType::Video)->params, video_options
    };

    /* create filter */
    constexpr auto accel { 3 };
    const std::vector<std::string> filters {
        "select='not(mod(n," + std::to_string(accel) + "))'"
        , "setpts=" + std::to_string(accel) + "*PTS"
    };
    fpp::LinearFilterGraph filter_graph {
        source.stream(fpp::MediaType::Video)->params
        , filters
    };

    /* create rescaler */
    fpp::RescaleContext rescaler {{
        source.stream(fpp::MediaType::Video)->params
        , sink.stream(fpp::MediaType::Video)->params
    }};

    /* open sink */
    if (!sink.open()) {
        return;
    }

    /* set read timeout if endless source stream */
    source.stream(fpp::MediaType::Video)->setEndTimePoint(60'000);

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_video_packet {
        [&input_packet,&source]() {
            do {
                input_packet = source.read();
            } while (!input_packet.isVideo() && !input_packet.isEOF());
            return !input_packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_video_packet()) {
        for (const auto& v_frame  : video_decoder.decode(input_packet)) {
        for (const auto& f_frame  : filter_graph.filter(v_frame))       {
        const auto r_frame { rescaler.scale(f_frame) };
        for (const auto& v_packet : video_encoder.encode(r_frame))      {
            sink.write(v_packet);
        }}}
    }

    /* explicitly close contexts */
    source.close();
    sink.close();
}
