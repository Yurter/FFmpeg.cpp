#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/RescaleContext.hpp>

void webcam_to_file() {

    /* create source */
    fpp::InputFormatContext source {
        "video=Webcam C170"
    };

    /* change default image size (480x360) */
    fpp::Options webcam_options {
        { "video_size", "1280x720" }
    };

    /* open source */
    source.open(/*webcam_options*/);

    /* create sink */
    fpp::OutputFormatContext sink {
        "webcam.flv"
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

    /* open sink */
    sink.open();

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_packet {
        [&input_packet,&source]() {
            input_packet = source.read();
            return !input_packet.isEOF();
        }
    };

    /* because of endless webcam's video */
//    source.stream(0)->setEndTimePoint(10 * 1000);

    /* read and write packets */
    while (read_packet()) {
        if (input_packet.isVideo()) {
            for (const auto& v_frame  : video_decoder.decode(input_packet)) {
            /*const*/ auto rv_frame { rescaler.scale(v_frame) };
//            rv_frame.raw().pict_type = AV_PICTURE_TYPE_NONE;
            for (const auto& v_packet : video_encoder.encode(rv_frame))     {
                sink.write(v_packet);
            }}
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
