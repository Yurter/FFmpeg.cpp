#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/resample/ResampleContext.hpp>
#include <fpp/scale/RescaleContext.hpp>
#include <fpp/core/Utils.hpp>
#include <fstream>

void rtp_video_and_audio_stream() {

    const std::string ip {
        "127.0.0.1"
    };

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create video sink */
    constexpr auto rtp_port_video  { 16700 };
    constexpr auto rtcp_port_video { rtp_port_video + 1 };
    fpp::OutputFormatContext video_sink {
        "rtp://" + ip + ":" + std::to_string(rtp_port_video)
            + "?rtcpport=" + std::to_string(rtcp_port_video)
    };

    /* copy only video stream to 1st sink */
    video_sink.copyStream(source.stream(fpp::Media::Type::Video));

    /* create audio sink */
    constexpr auto rtp_port_audio  { rtp_port_video + 2 };
    constexpr auto rtcp_port_audio { rtp_port_audio + 1 };
    fpp::OutputFormatContext audio_sink {
        "rtp://" + ip + ":" + std::to_string(rtp_port_audio)
            + "?rtcpport=" + std::to_string(rtcp_port_audio)
    };

    /* copy only audio stream to 2nd sink */
    audio_sink.copyStream(source.stream(fpp::Media::Type::Audio));
    /* fix audio stream index
     * (so that the player can distinguish video and audio stream) */
    audio_sink.stream(fpp::Media::Type::Audio)->setIndex(1);

    /* open sinks */
    if (!video_sink.open()) {
        return;
    }
    if (!audio_sink.open()) {
        return;
    }

    /* create sdp file */
    const auto sdp {
        fpp::utils::merge_sdp_files(
            video_sink.sdp()
            , audio_sink.sdp()
        )
    };
    std::ofstream sdp_file;
    sdp_file.open("video_and_audio.sdp");
    sdp_file << sdp;
    sdp_file.close();

    fpp::Packet packet;
    const auto read_packet {
        [&packet,&source]() {
            packet = source.read();
            return !packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        if (packet.isVideo()) {
            video_sink.write(packet);
        }
        else if (packet.isAudio()) {
            packet.setStreamIndex(0);
            audio_sink.write(packet);
        }
    }

    /* explicitly close contexts */
    source.close();
    video_sink.close();
    audio_sink.close();

}
