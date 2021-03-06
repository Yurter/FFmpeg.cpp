#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/scale/RescaleContext.hpp>
#include <fpp/filter/LinearFilterGraph.hpp>

void webcam_to_udp() {

    /* create source */
    fpp::InputFormatContext source {
        fpp::camera("HD WebCam")
    };

    /* change default image size (480x360) */
    fpp::Options webcam_options {
//        { "video_size", "1024x768" }
//        , { "framerate", "30" }
    };

    /* open source */
    if (!source.open(webcam_options)) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink {
          "udp://127.0.0.1:1234"
        , "mpegts"
    };

    /* encode video because of camera's rawvideo codec */
    const auto out_params { fpp::VideoParameters::make_shared() };
    out_params->setEncoder(AVCodecID::AV_CODEC_ID_H264);
    out_params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV420P);
    out_params->setGopSize(16);
    const auto in_params { source.stream(fpp::MediaType::Video)->params };
    out_params->completeFrom(in_params);

    /* create stream with predefined params */
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
        , { "crf",          "30"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };

    /* create encoders */
    fpp::EncoderContext video_encoder {
        sink.stream(fpp::MediaType::Video)->params, video_options
    };

    /* create rescaler (because of pixel format mismatch) */
    fpp::RescaleContext rescaler {{
        source.stream(fpp::MediaType::Video)->params
        , sink.stream(fpp::MediaType::Video)->params
    }};

    /* create fps filter (because of bug 'vlc and h264 variable framerate') */
    fpp::LinearFilterGraph graph {
        source.stream(fpp::MediaType::Video)->params
        , { "fps=fps=25", "setpts=400000*PTS" }
    };

    /* open sink */
    if (!sink.open()) {
        return;
    }

    fpp::Packet packet {
        fpp::MediaType::Unknown
    };
    const auto read_packet {
        [&packet,&source]() {
            packet = source.read();
            return !packet.isEOF();
        }
    };

    auto stop_flag { false }; // TODO: use lambda and return instead of flag and break (12.05)

    /* read and write packets */
    while (read_packet() && !stop_flag) {
        for (const auto& v_frame  : video_decoder.decode(packet))   {
        for (const auto& fv_frame : graph.filter(v_frame))          {
             const auto& rv_frame { rescaler.scale(fv_frame) };
        for (/*const*/ auto& v_packet : video_encoder.encode(rv_frame)) {
            v_packet.setStreamIndex(0);
            v_packet.setTimeBase(in_params->timeBase());
            stop_flag = !sink.write(v_packet);
        }}}
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
