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

void concatenate() { // TODO doesn't work 15.04

    /* create source */
    fpp::InputFormatContext source {
//        "concat:file0.flv|file1.flv"
        "mylist.txt"
    };

    /* open source */
    source.open();

    /* create sink */
    fpp::OutputFormatContext sink {
        "con.avi"
    };

    /* copy source's streams to sink */
    for (const auto& input_stream : source.streams()) {
        sink.copyStream(input_stream);
    }

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
