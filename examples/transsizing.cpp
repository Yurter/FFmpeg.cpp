#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/scale/RescaleContext.hpp>

void transsizing() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://91.197.91.139/live/ch00_0"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink {
        "transsized.flv"
    };

    const auto in_params  { source.stream(fpp::Media::Type::Video)->params };
    const auto out_params { fpp::VideoParameters::make_shared()          };

    /* resizing to 426x240 */
    out_params->setWidth(426);
    out_params->setHeight(240);
    out_params->completeFrom(in_params);

    /* create stream with predefined params */
    sink.createStream(out_params);

    /* create decoder */
    fpp::DecoderContext video_decoder {
        source.stream(fpp::Media::Type::Video)->params
    };

    /* create encoder's options */
    fpp::Options video_options {
          { "threads",      "1"           }
        , { "thread_type",  "slice"       }
        , { "preset",       "ultrafast"   }
        , { "crf",          "30"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };

    /* create encoders */
    fpp::EncoderContext video_encoder {
        sink.stream(fpp::Media::Type::Video)->params, video_options
    };

    /* create rescaler (because of pixel format mismatch) */
    fpp::RescaleContext rescaler {{
        source.stream(fpp::Media::Type::Video)->params
        , sink.stream(fpp::Media::Type::Video)->params
    }};

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

    /* because of endless stream */
    sink.stream(0)->setEndTimePoint(10 * 1000);

    auto stop_flag { false }; // TODO: use lambda and return instead of flag and break (12.05)

    /* read, resize and write packets */
    while (read_packet() && !stop_flag) {
        for (const auto& v_frame  : video_decoder.decode(packet)) {
            const auto& rv_frame { rescaler.scale(v_frame) };
            for (auto& v_packet : video_encoder.encode(rv_frame)) {
                stop_flag = !sink.write(v_packet);
            }
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
