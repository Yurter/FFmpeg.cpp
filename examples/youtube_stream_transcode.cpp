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

//void youtube_stream_transcode() {

//    /* create source */
//    fpp::InputFormatContext source {
//        "rtsp://87.197.138.187/live/ch00_0"
//    };

//    /* open source */
//    source.open();

//    /* check input streams */
//    if (!source.stream(fpp::MediaType::Video)) {
//        std::cout << "Youtube require video stream\n";
//    }
//    if (!source.stream(fpp::MediaType::Audio)) {
//        std::cout << "Youtube require video stream\n";
//    }

//    /* get input parameters */
//    const auto in_video_params {
//        source.stream(fpp::MediaType::Video)->params
//    };
//    const auto in_audio_params {
//        source.stream(fpp::MediaType::Audio)->params
//    };

//    /* create sink */
//    const std::string stream_key {
//        "w24s-620c-21kg-4vy7"
//    };
//    fpp::OutputFormatContext sink {
//        "rtmp://a.rtmp.youtube.com/live2/"
//        + stream_key
//    };

//    /* create streams with predefined params */
//    const auto out_video_params { fpp::utils::make_youtube_video_params() };
//    const auto out_audio_params { fpp::utils::make_youtube_audio_params() };
//    out_video_params->completeFrom(in_video_params);
//    out_audio_params->completeFrom(in_audio_params);
//    sink.createStream(out_video_params);
//    sink.createStream(out_audio_params);

//    /* create decoders */
//    fpp::DecoderContext video_decoder {
//        source.stream(fpp::MediaType::Video)->params
//    };
//    fpp::DecoderContext audio_decoder {
//        source.stream(fpp::MediaType::Audio)->params
//    };

//    /* create encoder's options */
//    fpp::Options video_options {
//          { "threads",      "1"           }
//        , { "thread_type",  "slice"       }
//        , { "preset",       "ultrafast"   }
//        , { "crf",          "30"          } // 0-51
////        , { "profile",      "main"        }
//        , { "tune",         "zerolatency" }
//    };

//    fpp::Options audio_options {
//        { "preset", "low" }
//    };

//    /* create encoders */
//    fpp::EncoderContext video_encoder {
//        sink.stream(fpp::MediaType::Video)->params, video_options
//    };
//    fpp::EncoderContext audio_encoder {
//        sink.stream(fpp::MediaType::Audio)->params, audio_options
//    };

//    /* create resampler */
//    fpp::ResampleContext resample {{
//        source.stream(fpp::MediaType::Audio)->params
//        , youtube.stream(fpp::MediaType::Audio)->params
//    }};

//    /* open sink */
//    youtube.open();

//    const auto video_transcoding_required {
//        fpp::utils::transcoding_required({
//            source.stream(fpp::MediaType::Video)->params
//            , youtube.stream(fpp::MediaType::Video)->params
//        })
//    };

//    fpp::Packet input_packet {
//        fpp::MediaType::Unknown
//    };
//    const auto read_packet {
//        [&input_packet,&source]() {
//            input_packet = source.read();
//            return !input_packet.isEOF();
//        }
//    };

//    /* read and write packets */
//    while (read_packet()) {
//        if (input_packet.isVideo()) {
//            if (video_transcoding_required) {
//                for (const auto& v_frame  : video_decoder.decode(input_packet)) {
//                for (const auto& v_packet : video_encoder.encode(v_frame))      {
//                    youtube.write(v_packet);
//                }}
//            }
//            else {
//                youtube.write(input_packet);
//            }
//        }
//        else if (input_packet.isAudio()) {
//            for (const auto& a_frame  : audio_decoder.decode(input_packet)) {
//            for (const auto& ra_frame : resample.resample(a_frame))         {
//            for (const auto& a_packet : audio_encoder.encode(ra_frame))     {
//                youtube.write(a_packet);
//            }}}
//        }
//    }

//    /* explicitly close contexts */
//    source.close();
//    youtube.close();

//}
