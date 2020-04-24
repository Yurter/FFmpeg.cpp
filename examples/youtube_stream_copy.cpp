#include "examples.hpp"

#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>

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

    // TODO: remove (23.04)
    sink.stream(0)->params->setExtradata(source.stream(0)->params->extradata());
    sink.stream(1)->params->setExtradata(source.stream(1)->params->extradata());

    /* open sink */
    sink.open();

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
