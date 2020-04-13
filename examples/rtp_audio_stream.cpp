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

void rtp_audio_stream() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    source.open();

    /* create sink */
    const std::string ip { "127.0.0.1" };
    const auto rtp_port  { 16700 };
    const auto rtcp_port { rtp_port + 1 };
    fpp::OutputFormatContext rtp_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port)
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* copy only audio stream to sink */
    rtp_restreamer.copyStream(source.stream(fpp::MediaType::Audio));

    /* open sink */
    rtp_restreamer.open();

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("audio.sdp");
    sdp_file << rtp_restreamer.sdp();
    sdp_file.close();

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_audio_packet {
        [&input_packet,&source]() {
            do {
                input_packet = source.read();
            } while (!input_packet.isAudio() && !input_packet.isEOF());
            return !input_packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_audio_packet()) {
        /* fix audio packet's stream index */
        input_packet.setStreamIndex(0);
        rtp_restreamer.write(input_packet);
    }

    /* explicitly close contexts */
    source.close();
    rtp_restreamer.close();

}
