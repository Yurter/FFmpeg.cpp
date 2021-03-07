#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>

#include <fpp/core/Utils.hpp>
#include <fpp/stream/VideoParameters.hpp>

void read_and_write_to_memory() {

    /* create source */
    fpp::InputFormatContext real_source {
        "rtsp://205.120.142.79/live/ch00_0"
    };

    /* open source */
    if (!real_source.open()) {
        return;
    }

    std::uint8_t buffer[4096 * 50];
    std::size_t buffer_size { 0 };

    /* create custom output buffer */
    fpp::OutputContext custom_output_buffer {
        [&](const std::uint8_t* buf, std::size_t buf_size) {
            fpp::static_log_info() << "Buffer size:" << buffer_size;
            fpp::static_log_info() << "Write" << buf_size << "bytes to memory";
            std::memcpy(buffer + buffer_size, buf, buf_size);
            buffer_size += buf_size;
            return true;
        }
    };

    /* create sink */
    fpp::OutputFormatContext memory_sink {
          &custom_output_buffer
        , "flv"
    };

    /* copy source's streams to sink */
    for (const auto& input_stream : real_source.streams()) {
        memory_sink.copyStream(input_stream);
    }

    /* open sink */
    if (!memory_sink.open()) {
        return;
    }

    /* create custom input buffer */
    fpp::InputContext custom_input_buffer {
        [&](std::uint8_t* buf, std::size_t buf_size) -> fpp::InputContext::CbResult {
            fpp::static_log_info() << "Buffer size:" << buffer_size;
            const auto bytesRead { std::min(buf_size, buffer_size) };
            std::memcpy(buf + buffer_size - bytesRead, buffer, bytesRead);
            buffer_size -= bytesRead;
            fpp::static_log_info() << "Read" << bytesRead << "bytes from memory";
            return { true, bytesRead };
        }
    };

    /* ? */
    fpp::InputFormatContext memory_source {
          &custom_input_buffer
        , "flv"
    };

    /* ? */
    if (!memory_source.open()) {
        return;
    }

    std::static_pointer_cast<fpp::VideoParameters>(memory_source.stream(0)->params)->setWidth(1280);
    std::static_pointer_cast<fpp::VideoParameters>(memory_source.stream(0)->params)->setHeight(720);

    /* ? */
    fpp::OutputFormatContext real_sink {
        "output.flv"
    };

    /* ? */
    for (const auto& input_stream : memory_source.streams()) {
        real_sink.copyStream(input_stream);
    }

    /* ? */
    if (!real_sink.open()) {
        return;
    }

    while (true) {
        if (auto packet { real_source.read() }; packet.isEOF()) { break; }
        else { if (!memory_sink.write(packet)) { break; } }

        if (auto packet { memory_source.read() }; packet.isEOF()) { break; }
        else { if (!real_sink.write(packet)) { break; } }
    }

    /* explicitly close contexts */
    real_sink.close();
    real_source.close();
    memory_sink.close();
    memory_source.close();

}
