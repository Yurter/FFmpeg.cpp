#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/resample/ResampleContext.hpp>
#include <fpp/scale/RescaleContext.hpp>
#include <fpp/core/Utils.hpp>

void youtube_stream_transcode_with_silence() {

    /* create video source */
    fpp::InputFormatContext video_source {
        "desktop"
    };

    /* open video source */
    if (!video_source.open()) {
        return;
    }

    /* check input video stream */
    if (!video_source.stream(fpp::MediaType::Video)) {
        fpp::static_log_error(__func__, "Youtube require video stream");
        return;
    }

    /* create audio source */
    fpp::InputFormatContext audio_source {
        fpp::InputFormatContext::silence(44'100)
    };

    /* open audio source */
    if (!audio_source.open()) {
        return;
    }

    /* create sink */
    const std::string stream_key {
        "aaaa-bbbb-cccc-dddd"
    };
    fpp::OutputFormatContext sink {
        "rtmp://a.rtmp.youtube.com/live2/"
        + stream_key
    };

    /* create output params */
    const auto in_video_params { video_source.stream(fpp::MediaType::Video)->params };
    const auto in_audio_params { audio_source.stream(fpp::MediaType::Audio)->params };
    const auto out_video_params { fpp::utils::make_youtube_video_params() };
    const auto out_audio_params { fpp::utils::make_youtube_audio_params() };
    out_video_params->completeFrom(in_video_params);
    out_audio_params->completeFrom(in_audio_params);

    /* create output streams */
    sink.createStream(out_video_params);
    sink.createStream(out_audio_params);

    /* create video codec contexts and rescaler */
    fpp::DecoderContext video_decoder {
        video_source.stream(fpp::MediaType::Video)->params
    };
    fpp::RescaleContext rescaler {{
        video_source.stream(fpp::MediaType::Video)->params
        , sink.stream(fpp::MediaType::Video)->params
    }};
    fpp::Options video_options {
          { "threads",      "1"           }
        , { "thread_type",  "slice"       }
        , { "preset",       "ultrafast"   }
        , { "crf",          "30"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };
    fpp::EncoderContext video_encoder {
        sink.stream(fpp::MediaType::Video)->params, video_options
    };

    /* because of pcm_u8 anullsrc's codec */
    fpp::DecoderContext audio_decoder {
        audio_source.stream(fpp::MediaType::Audio)->params
    };
    fpp::ResampleContext resample {{
        audio_source.stream(fpp::MediaType::Audio)->params
        , sink.stream(fpp::MediaType::Audio)->params
    }};
    fpp::EncoderContext audio_encoder {
        sink.stream(fpp::MediaType::Audio)->params
    };

    /* because of non zero dshow start stamps
     * and zero lavfi start stamps */
    video_source.stream(fpp::MediaType::Video)->stampFromZero(true);

    /* open sink */
    if (!sink.open()) {
        return;
    }

    /* stamp values for video/audio sync */
    auto last_video_dts { 0ll };
    auto last_audio_dts { 0ll };

    /* convert dts from packet timebase to ms */
    const auto dts2ms {
        [](const fpp::Packet& packet) {
            return ::av_rescale_q(packet.dts(), packet.timeBase(), DEFAULT_TIME_BASE);
        }
    };

    fpp::Packet packet {
        fpp::MediaType::Unknown
    };
    const auto read_packet {
        [&]() {
            if (last_video_dts > last_audio_dts) {
                packet = audio_source.read();
                last_audio_dts = dts2ms(packet);
            }
            else {
                packet = video_source.read();
                last_video_dts = dts2ms(packet);
            }
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
    video_source.close();
    audio_source.close();
    sink.close();

}
