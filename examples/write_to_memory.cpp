#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/core/Logger.hpp>
#include <fstream>

void write_to_memory() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://205.120.142.79/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    constexpr auto outputFileName { "outputFile" };
    auto file { std::fstream(outputFileName, std::ios::out | std::ios::binary) };
    if (!file.is_open()) {
        fpp::static_log_error() << "Failed to open output file: " << outputFileName;
        return;
    }

    /* create custom output buffer */
    fpp::OutputContext custom_buffer {
        [&file](const std::uint8_t* buf, std::size_t buf_size) {
            fpp::static_log_info() << "Write" << buf_size << "bytes";
            file.write(
                  reinterpret_cast<const char*>(buf)
                , static_cast<std::streamsize>(buf_size)
            );
            return true;
        }
    };

    /* create sink */
    fpp::OutputFormatContext sink {
          &custom_buffer
        , "flv"
    };

    /* copy source's streams to sink */
    for (const auto& input_stream : source.streams()) {
        sink.copyStream(input_stream);
    }

    /* open sink */
    if (!sink.open()) {
        return;
    }

    fpp::Packet packet;
    const auto read_packet {
        [&packet,&source]() {
            packet = source.read();
            return !packet.isEOF();
        }
    };

    /* set timeout (because of endless rtsp stream) */
    constexpr auto thirty_seconds { 30 * 1000 };
    source.stream(0)->setEndTimePoint(thirty_seconds);

    /* read and write packets */
    while (read_packet()) {
        sink.write(packet);
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

    /* explicitly close output file */
    file.close();

}
