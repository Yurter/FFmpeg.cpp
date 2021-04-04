#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>

void read_from_memory() {

//    /* create source */
//    fpp::InputFormatContext source {
//        "rtsp://205.120.142.79/live/ch00_0"
//    };

//    /* open source */
//    if (!source.open()) {
//        return;
//    }

//    /* create custom output buffer */
//    fpp::OutputContext custom_buffer {
//        [](const std::uint8_t* /*buf*/, std::size_t buf_size) {
//            fpp::static_log_info() << "Write" << buf_size << "bytes";
//            return true;
//        }
//    };

//    /* create sink */
//    fpp::OutputFormatContext sink {
//          &custom_buffer
//        , "flv"
//    };

//    /* copy source's streams to sink */
//    for (const auto& input_stream : source.streams()) {
//        sink.copyStream(input_stream);
//    }

//    /* open sink */
//    if (!sink.open()) {
//        return;
//    }

//    fpp::Packet packet;
//    const auto read_packet {
//        [&packet,&source]() {
//            packet = source.read();
//            return !packet.isEOF();
//        }
//    };

//    /* read and write packets */
//    while (read_packet()) {
//        sink.write(packet);
//    }

//    /* explicitly close contexts */
//    source.close();
//    sink.close();

}
