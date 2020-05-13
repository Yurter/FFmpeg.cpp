#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/scale/RescaleContext.hpp>

void record_screen_win() {

    /* create source */
    fpp::InputFormatContext source {
        "desktop"
    };

    fpp::Options source_options {
        { "framerate", "ntsc" }
        , { "offset_x", "100" }       // remove this options
        , { "offset_y", "100" }       // for recording
        , { "video_size", "192x108" } // all available screen area
    };

    /* open source */
    if (!source.open(source_options)) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink {
        "desktop.flv"
    };

    /* encode video because of desktop's bmp codec */
    const auto out_params { fpp::VideoParameters::make_shared() };
    out_params->setEncoder(AVCodecID::AV_CODEC_ID_H264);
    out_params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV420P);
    out_params->setGopSize(12);
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

    /* because of endless webcam's video */
    constexpr auto one_minute { 1 * 60 * 1000 };
    source.stream(0)->setEndTimePoint(one_minute);

    /* read and write packets */
    while (read_packet()) {
        if (packet.isVideo()) {
            for (const auto& v_frame  : video_decoder.decode(packet))   {
                 const auto& rv_frame { rescaler.scale(v_frame) };
            for (const auto& v_packet : video_encoder.encode(rv_frame)) {
                sink.write(v_packet);
            }}
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
