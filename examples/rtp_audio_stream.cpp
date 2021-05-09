#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/resample/ResampleContext.hpp>
#include <fstream>

void rtp_audio_stream() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    const std::string ip { "127.0.0.1" };
    constexpr auto rtp_port { 16700 };
    constexpr auto rtcp_port { rtp_port + 1 };
    fpp::OutputFormatContext sink {
        "rtp://" + ip + ":" + std::to_string(rtp_port)
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* copy only audio stream to sink */
    sink.copyStream(source.stream(fpp::Media::Type::Audio));

    /* open sink */
    if (!sink.open()) {
        return;
    }

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("audio.sdp");
    sdp_file << sink.sdp();
    sdp_file.close();

    fpp::Packet packet;
    const auto read_packet {
        [&packet,&source]() {
            packet = source.read();
            return !packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_packet()) {
        if (packet.isAudio()) {
            /* fix audio packet's stream index */
            packet.setStreamIndex(0);
            sink.write(packet);
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
