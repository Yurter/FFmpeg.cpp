#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <array>

void multiple_outputs_parallel() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://213.129.131.54/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    constexpr auto N { 5 };
    std::array<fpp::OutputFormatContext,N> sinks;

    for (std::size_t i { 0 }; i < N; ++i) {
        const auto file_name { std::to_string(i).append(".flv") }; // 0.flv, 1.flv...
        sinks[i].setMediaResourceLocator(file_name);
        sinks[i].copyStream(source.stream(fpp::Media::Type::Video));
        if (!sinks[i].open()) {
            return;
        }
    }

    fpp::Packet packet;
    const auto read_packet {
        [&packet,&source]() {
            packet = source.read();
            return !packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        for (auto& sink : sinks) {
            sink.write(packet);
        }
    }

    /* explicitly close context */
    source.close();
    for (auto& sink : sinks) {
        sink.close();
    }

}
