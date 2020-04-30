#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>

void multiple_outputs_sequence() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://213.129.131.54/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink;

    constexpr auto N { 5 };
    for (auto i { 0 }; i < N; ++i) {

        /* set next file name */
        const auto file_name { std::to_string(i).append(".flv") }; // 0.flv, 1.flv...
        sink.setMediaResourceLocator(file_name);

        /* copy source video stream to sink */
        sink.copyStream(source.stream(fpp::MediaType::Video));

        /* open sink */
        if (!sink.open()) {
            return;
        }

        fpp::Packet packet {
            fpp::MediaType::Unknown
        };
        auto read_video_packet_n { // read n packets, start from key frame
            [&packet
            ,&source
            ,gotkey_frame=false
            ,counter=0
            ](int n) mutable {
                if (!gotkey_frame) { // prevent start from non key frame
                    do {
                        packet = source.read();
                    } while (!packet.keyFrame());
                    gotkey_frame = true;
                } else {
                    packet = source.read();
                }
                if (++counter == n) {
                    return false;
                }
                return !packet.isEOF();
            }
        };

        /* read and write packets */
        while (read_video_packet_n(200)) {
            sink.write(packet);
        }

        /* explicitly close context */
        sink.close();

    }

    /* explicitly close context */
    source.close();

}
