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

void youtube_stream_copy() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://87.197.138.187/live/ch00_0"
    };

    /* open source */
    source.open();

    /* check input streams */
    if (!source.stream(fpp::MediaType::Video)) {
        std::cout << "Youtube require video stream\n";
    }
    if (!source.stream(fpp::MediaType::Audio)) {
        std::cout << "Youtube require video stream\n";
    }

    /* create sink */
    const std::string stream_key {
        "aaaa-bbbb-cccc-dddd"
    };
    fpp::OutputFormatContext sink {
        "rtmp://a.rtmp.youtube.com/live2/"
        + stream_key
    };

    /* copy source's streams to sink */
    sink.copyStream(source.stream(fpp::MediaType::Video));
    sink.copyStream(source.stream(fpp::MediaType::Audio));

    /* open sink */
    sink.open();

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
        sink.write(input_packet);
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
