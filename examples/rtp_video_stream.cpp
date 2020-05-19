#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fstream>

void rtp_video_stream() {

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
        "rtp://" + ip + ":" + std::to_string(rtp_port) // TODO add local optional 31.03 (localrtpport, localrtcpport) -> https://www.ffmpeg.org/ffmpeg-protocols.html#rtp
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* copy only video stream to sink */
    sink.copyStream(source.stream(fpp::MediaType::Video));

    /* open sink */
    if (!sink.open()) {
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
    const auto read_packet {
        [&packet,&source]() {
            packet = source.read();
            return !packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_packet()) {
        if (packet.isVideo()) {
            sink.write(packet);
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
