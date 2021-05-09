#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/scale/RescaleContext.hpp>
#include <fpp/filter/LinearFilterGraph.hpp>
#include <fpp/refi/VideoFilters/DrawText.hpp>

void text_on_video() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink {
        "text_on_video.flv"
    };

    /* copy only video stream to sink */
    sink.copyStream(source.stream(fpp::Media::Type::Video));

    /* create decoder */
    fpp::DecoderContext video_decoder {
        source.stream(fpp::Media::Type::Video)->params
    };

    /* encoder's options */
    fpp::Options video_options {
          { "threads",      "1"           }
        , { "thread_type",  "slice"       }
        , { "preset",       "ultrafast"   }
        , { "crf",          "30"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };

    /* create encoder */
    fpp::EncoderContext video_encoder {
        sink.stream(fpp::Media::Type::Video)->params, video_options
    };

    const auto draw_text {
        fpp::VideoFilter::DrawText::make(
            fpp::VideoFilter::DrawText::Text {
                "Hello World"   /* text      */
                , ""            /* text file */
                , "100"         /* x         */
                , "100"         /* y         */
            }
            , fpp::VideoFilter::DrawText::Font {
                ""              /* font */
                , ""            /* font file */
                , "36"          /* font size */
                , "red"         /* color */
                , ""            /* fontcolor_expr */
            }
        )
    };

    /* create filter */
    fpp::LinearFilterGraph filter_graph {
        source.stream(fpp::Media::Type::Video)->params
        , { draw_text }
    };

    /* open sink */
    if (!sink.open()) {
        return;
    }

    /* set timeout (because of endless rtsp stream) */
    constexpr auto one_minute { 1 * 60 * 1000 };
    source.stream(fpp::Media::Type::Video)->setEndTimePoint(one_minute);

    fpp::Packet packet;
    const auto read_packet {
        [&packet,&source]() {
            packet = source.read();
            return !packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        if (packet.isVideo()) {
            for (const auto& v_frame  : video_decoder.decode(packet))  {
            for (const auto& f_frame  : filter_graph.filter(v_frame))  {
            for (      auto& v_packet : video_encoder.encode(f_frame)) {
                sink.write(v_packet);
            }}}
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
