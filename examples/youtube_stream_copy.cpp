#include "examples.hpp"

#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>

void youtube_stream_copy() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://87.197.138.187/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* check input streams */
    if (!source.stream(fpp::MediaType::Video)) {
        fpp::static_log_error(__func__, "Youtube require video stream");
    }
    if (!source.stream(fpp::MediaType::Audio)) {
        fpp::static_log_error(__func__, "Youtube require video stream");
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
    if (!sink.open()) {
        return;
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
        sink.write(packet);
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
