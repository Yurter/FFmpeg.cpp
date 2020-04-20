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

void rtp_video_stream() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://205.120.142.79/live/ch00_0"
    };

    /* open source */
    source.open();

    /* create sink */
    const std::string ip { "127.0.0.1" };
    const auto rtp_port  { 16700 };
    const auto rtcp_port { rtp_port + 1 };

    fpp::OutputFormatContext rtp_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port) // TODO add local optional 31.03 (localrtpport, localrtcpport) -> https://www.ffmpeg.org/ffmpeg-protocols.html#rtp
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* copy only video stream to sink */
    rtp_restreamer.copyStream(source.stream(fpp::MediaType::Video));

    /* open sink */
    rtp_restreamer.open();

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("video.sdp");
    sdp_file << rtp_restreamer.sdp();
    sdp_file.close();

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_video_packet {
        [&input_packet,&source]() {
            do {
                input_packet = source.read();
            } while (!input_packet.isVideo() && !input_packet.isEOF());
            return !input_packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_video_packet()) {
        rtp_restreamer.write(input_packet);
    }

    /* explicitly close contexts */
    source.close();
    rtp_restreamer.close();

}
