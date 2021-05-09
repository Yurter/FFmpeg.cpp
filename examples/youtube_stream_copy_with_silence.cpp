#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/resample/ResampleContext.hpp>
#include <fpp/core/Utils.hpp>

void youtube_stream_copy_with_silence() {

    /* create video source */
    fpp::InputFormatContext video_source {
        "rtsp://205.120.142.79/live/ch00_0"
    };

    /* open video source */
    if (!video_source.open()) {
        return;
    }

    /* check input video stream */
    if (!video_source.stream(fpp::Media::Type::Video)) {
        fpp::static_log_error() << "Youtube require video stream";
        return;
    }

    /* create audio source */
    fpp::InputFormatContext audio_source {
        "anullsrc=r=" + std::to_string(44'100) + ":cl=mono"
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

    /* copy source's video stream to sink */
    sink.copyStream(video_source.stream(fpp::Media::Type::Video));

    /* create output audio params from anullsrc's */
    const auto in_audio_params {
        audio_source.stream(fpp::Media::Type::Audio)->params
    };
    const auto out_audio_params {
        fpp::utils::make_youtube_audio_params()
    };
    out_audio_params->completeFrom(in_audio_params);

    /* create output audio stream */
    sink.createStream(out_audio_params);

    /* because of pcm_u8 anullsrc's codec */
    fpp::DecoderContext audio_decoder {
        audio_source.stream(fpp::Media::Type::Audio)->params
    };
    fpp::ResampleContext resample {{
          audio_source.stream(fpp::Media::Type::Audio)->params
        , sink.stream(fpp::Media::Type::Audio)->params
    }};
    fpp::EncoderContext audio_encoder {
        sink.stream(fpp::Media::Type::Audio)->params
    };

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

    fpp::Packet packet;
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
            sink.write(packet);
        }
        else if (packet.isAudio()) {
            for (const auto& a_frame  : audio_decoder.decode(packet))   {
            for (const auto& ra_frame : resample.resample(a_frame))     {
            for (      auto& a_packet : audio_encoder.encode(ra_frame)) {
                sink.write(a_packet);
            }}}
        }
    }

    /* explicitly close contexts */
    video_source.close();
    audio_source.close();
    sink.close();

}
