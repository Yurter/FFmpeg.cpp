#include <iostream>
#include <fstream>
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/ResampleContext.hpp>
#include <fpp/refi/RescaleContext.hpp>
#include <fpp/refi/VideoFilterContext.hpp>
#include <fpp/refi/VideoFilters/DrawText.hpp>
#include <fpp/core/Utils.hpp>
#include "examples.hpp"

//void webcam_to_file() {

//    /* create source */
//    fpp::InputFormatContext source {
//        "video=Webcam C170"
////        "rtsp://90.80.246.160/live/ch00_0"
//    };

//    /* change default image size (480x360) */
//    fpp::Options webcam_options {
////        { "video_size", "1280x720" }
//         { "framerate",      "30"        }
//        , { "threads", "1" }
//    };

//    /* open source */
//    webcam.open(webcam_options);

//    /* create sink */
//    fpp::OutputFormatContext video_file {
//        "webcam.flv"
////        "webcam.mp4"
//    };

//    /* encode video because of camera's rawvideo codec */
//    const auto out_params { fpp::VideoParameters::make_shared() };
////    out_params->setEncoder(AVCodecID::AV_CODEC_ID_FLV1);
//    out_params->setEncoder(AVCodecID::AV_CODEC_ID_H264);
//    out_params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV420P);
////    out_params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV422P);
//    out_params->setGopSize(16);
////    out_params->setBitrate(1000);
//    const auto in_params { source.stream(fpp::MediaType::Video)->params };
//    out_params->completeFrom(in_params);

//    /* create stream with predefined params */
//    sink.createStream(out_params);

//    /* create decoder */
//    fpp::DecoderContext video_decoder {
//        webcam.stream(fpp::MediaType::Video)->params
//    };

//    /* create encoder's options */
//    fpp::Options video_options {
////          { "threads",      "1"           }
////        , { "thread_type",  "slice"       }
////        , { "preset",       "ultrafast"   }
////        , { "crf",          "30"          } // 0-51
//        /*,*/
////          { "profile",      "main"        }
////         { "profile",      "high"        }
////        { "profile",      "high422"        }
////        { "bf",      "2"        }

////        { "profile",      "baseline"        }
//        //        , { "tune",         "zerolatency" }
//    };

//    /* create encoders */
//    fpp::EncoderContext video_encoder {
//        video_file.stream(fpp::MediaType::Video)->params, video_options
//    };

//    /* create rescaler (because of pixel format mismatch) */
//    fpp::RescaleContext rescaler {{
//        webcam.stream(fpp::MediaType::Video)->params
//        , video_file.stream(fpp::MediaType::Video)->params
//    }};

//    /* open sink */
//    video_file.open();

//    fpp::Packet input_packet {
//        fpp::MediaType::Unknown
//    };
//    const auto read_packet {
//        [&input_packet,&webcam]() {
//            input_packet = webcam.read();
//            return !input_packet.isEOF();
//        }
//    };

//    /* because of endless webcam's video */
////    source.stream(0)->setEndTimePoint(10 * 1000);

//    auto counter { 0 };

//    /* read and write packets */
//    while (read_video_packet()) {
//        if (counter++ == 500) {
//            break;
//        }
//        if (input_packet.isVideo()) {
//            for (const auto& v_frame  : video_decoder.decode(input_packet)) {
//            /*const*/ auto rv_frame { rescaler.scale(v_frame) };
////            rv_frame.raw().pict_type = AV_PICTURE_TYPE_NONE;
//            for (const auto& v_packet : video_encoder.encode(rv_frame))     {
//                video_file.write(v_packet);
//            }}
//        }
//    }

//    /* explicitly close contexts */
//    webcam.close();
//    video_file.close();

//}
