#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>

void transmuxing_file() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://205.120.142.79/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink {
        "transmuxing.flv"
    };

    /* copy source's streams to sink */
    for (const auto& input_stream : source.streams()) {
        sink.copyStream(input_stream);
    }

    /* open sink */
    if (!sink.open()) {
        return;
    }

    /* set timeout (because of endless rtsp stream) */
    constexpr auto one_minute { 1 * 60 * 1000 };
    source.stream(0)->setEndTimePoint(one_minute);

    fpp::Packet packet {
        fpp::MediaType::Unknown
    };
    const auto read_packet {
        [&packet,&source]() {
            packet = source.read();
            return !packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        sink.write(packet);
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
