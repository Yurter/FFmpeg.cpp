#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/RescaleContext.hpp>

void rtp_video_stream_transcoded() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://205.120.142.79/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    const std::string ip { "127.0.0.1" };
    constexpr auto rtp_port  { 16700 };
    constexpr auto rtcp_port { rtp_port + 1 };

    fpp::OutputFormatContext sink {
        "rtp://" + ip + ":" + std::to_string(rtp_port)
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* create stream with predefined params */
    const auto out_params { fpp::VideoParameters::make_shared() };
    out_params->setEncoder(AVCodecID::AV_CODEC_ID_H264);
    out_params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV420P);
    out_params->setGopSize(12);
    out_params->setTimeBase(DEFAULT_TIME_BASE);
    out_params->completeFrom(source.stream(fpp::MediaType::Video)->params);

    sink.createStream(out_params);

    /* create decoder */
    fpp::DecoderContext video_decoder {
        source.stream(fpp::MediaType::Video)->params
    };

    /* create encoder's options */
    fpp::Options video_options {
        { "threads",        "1"          }
        , { "thread_type",  "slice"       }
        , { "preset",       "ultrafast"   }
        , { "crf",          "15"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };

    /* create encoder */
    fpp::EncoderContext video_encoder {
        sink.stream(fpp::MediaType::Video)->params, video_options
    };

    /* create rescaler */
    fpp::RescaleContext rescaler {{
        source.stream(fpp::MediaType::Video)->params
        , sink.stream(fpp::MediaType::Video)->params
    }};

    /* open sink */
    if (sink.open()) {
        return;
    }

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("video.sdp");
    sdp_file << sink.sdp();
    sdp_file.close();

    fpp::Packet packet {
        fpp::MediaType::Unknown
    };
    const auto read_video_packet {
        [&packet,&source]() {
            do {
                packet = source.read();
            } while (!packet.isVideo() && !packet.isEOF());
            return !packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_video_packet()) {
        for (const auto& v_frame  : video_decoder.decode(packet))  {
             const auto& rv_frame { rescaler.scale(v_frame) };
        for (const auto& v_packet : video_encoder.encode(rv_frame)) {
            sink.write(v_packet);
        }}
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
