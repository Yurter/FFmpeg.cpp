#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/core/Logger.hpp>
#include <fstream>

void read_from_memory() {

    constexpr auto inputFileName { "file" };
    std::ifstream file { inputFileName, std::ios::out | std::ios::binary };
    if (!file.is_open()) {
        fpp::static_log_error() << "Failed to open input file: " << inputFileName;
        return;
    }

    /* create custom input buffer */
    fpp::InputContext custom_input_buffer {
        [&file](std::uint8_t* buf, std::size_t buf_size) -> fpp::InputContext::CbResult {
            if (!file.read(reinterpret_cast<char*>(buf), static_cast<std::streamsize>(buf_size))) {
                return {};
            }
            const auto bytesRead { file.gcount() };
            fpp::static_log_info() << "Read" << bytesRead << "bytes from memory";
            return { true, static_cast<std::size_t>(bytesRead) };
        }
    };

    /* create open source */
    fpp::InputFormatContext source {
          &custom_input_buffer
        , "avi"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink {
        "output.flv"
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
            fpp::static_log_info() << "Read" << packet.toString();
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
