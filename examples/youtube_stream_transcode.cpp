#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/resample/ResampleContext.hpp>
#include <fpp/scale/RescaleContext.hpp>
#include <fpp/core/Utils.hpp>

void youtube_stream_transcode() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://87.197.138.187/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* check input streams */
    if (!source.stream(fpp::MediaType::Video)) {
        fpp::static_log_error(__func__, "Youtube require video stream");
    }
    if (!source.stream(fpp::MediaType::Audio)) {
        fpp::static_log_error(__func__, "Youtube require video stream");
    }

    /* get input parameters */
    const auto in_video_params {
        source.stream(fpp::MediaType::Video)->params
    };
    const auto in_audio_params {
        source.stream(fpp::MediaType::Audio)->params
    };

    /* create sink */
    const std::string stream_key {
        "aaaa-bbbb-cccc-dddd"
    };
    fpp::OutputFormatContext sink {
        "rtmp://a.rtmp.youtube.com/live2/"
        + stream_key
    };

    /* create streams with predefined params */
    const auto out_video_params { fpp::utils::make_youtube_video_params() };
    const auto out_audio_params { fpp::utils::make_youtube_audio_params() };
    out_video_params->completeFrom(in_video_params);
    out_audio_params->completeFrom(in_audio_params);
    sink.createStream(out_video_params);
    sink.createStream(out_audio_params);

    /* create decoders */
    fpp::DecoderContext video_decoder {
        source.stream(fpp::MediaType::Video)->params
    };
    fpp::DecoderContext audio_decoder {
        source.stream(fpp::MediaType::Audio)->params
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
    fpp::EncoderContext audio_encoder {
        sink.stream(fpp::MediaType::Audio)->params
    };

    /* create rescaler */
    fpp::RescaleContext rescaler {{
        source.stream(fpp::MediaType::Video)->params
        , sink.stream(fpp::MediaType::Video)->params
    }};

    /* create resampler */
    fpp::ResampleContext resample {{
        source.stream(fpp::MediaType::Audio)->params
        , sink.stream(fpp::MediaType::Audio)->params
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

    /* read and write packets */
    while (read_packet()) {
        if (packet.isVideo()) {
            for (const auto& v_frame  : video_decoder.decode(packet))   {
                 const auto& rv_frame { rescaler.scale(v_frame) };
            for (const auto& v_packet : video_encoder.encode(rv_frame)) {
                sink.write(v_packet);
            }}
        }
        else if (packet.isAudio()) {
            for (const auto& a_frame  : audio_decoder.decode(packet))   {
            for (const auto& ra_frame : resample.resample(a_frame))     {
            for (const auto& a_packet : audio_encoder.encode(ra_frame)) {
                sink.write(a_packet);
            }}}
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
