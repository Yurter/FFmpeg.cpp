#include "examples.hpp"

#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
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

    for (size_t i { 0 }; i < N; ++i) {
        sinks[i].setMediaResourceLocator(std::to_string(i).append(".flv"));
        sinks[i].copyStream(source.stream(fpp::MediaType::Video));
        sinks[i].stream(0)->params->setExtradata(source.stream(0)->params->extradata()); // TODO: remove (23.04)
        if (!sinks[i].open()) { return; }
    }

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
        for (size_t i { 0 }; i < N; ++i) {
            sinks[i].write(packet);
        }
    }

    /* explicitly close context */
    source.close();

}
