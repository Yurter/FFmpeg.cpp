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

void rtp_video_and_audio_stream() {

    const std::string ip {
        "127.0.0.1"
    };

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    source.open();

    /* create video sink */
    const auto rtp_port_video  { 16700 };
    const auto rtcp_port_video { rtp_port_video + 1 };
    fpp::OutputFormatContext rtp_video_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port_video)
            + "?rtcpport=" + std::to_string(rtcp_port_video)
    };

    /* copy only video stream to 1st sink */
    rtp_video_restreamer.copyStream(source.stream(fpp::MediaType::Video));

    /* create audio sink */
    const auto rtp_port_audio  { rtp_port_video + 2 };
    const auto rtcp_port_audio { rtp_port_audio + 1 };
    fpp::OutputFormatContext rtp_audio_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port_audio)
            + "?rtcpport=" + std::to_string(rtcp_port_audio)
    };

    /* copy only audio stream to 2nd sink */
    rtp_audio_restreamer.copyStream(source.stream(fpp::MediaType::Audio));
    /* fix audio stream index
     * (so that the player can distinguish video and audio stream) */
    rtp_audio_restreamer.stream(fpp::MediaType::Audio)->setIndex(1);

    /* open sinks */
    rtp_video_restreamer.open();
    rtp_audio_restreamer.open();

    /* create sdp file */
    const auto sdp {
        fpp::utils::merge_sdp_files(
            rtp_video_restreamer.sdp()
            , rtp_audio_restreamer.sdp()
        )
    };
    std::ofstream sdp_file;
    sdp_file.open("video_and_audio.sdp");
    sdp_file << sdp;
    sdp_file.close();

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_packet {
        [&input_packet,&source]() {
            input_packet = source.read();
            return !input_packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        if (input_packet.isVideo()) {
            rtp_video_restreamer.write(input_packet);
        }
        else if (input_packet.isAudio()) {
            input_packet.setStreamIndex(0);
            rtp_audio_restreamer.write(input_packet);
        }
    }

    /* explicitly close contexts */
    source.close();
    rtp_video_restreamer.close();
    rtp_audio_restreamer.close();

}
