#include <iostream>
#include <fstream>
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/ResampleContext.hpp>
#include <fpp/refi/RescaleContext.hpp>
#include <fpp/refi/VideoFilterContext.hpp>
#include <fpp/refi/VideoFilters/DrawText.hpp>
#include <fpp/core/Utils.hpp>
#include "examples.hpp"

void text_on_video() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    source.open();

    /* create sink */
    fpp::OutputFormatContext sink {
        "text_on_video.flv"
    };

    /* copy only video stream to sink */
    for (const auto& input_stream : source.streams()) {
        if (input_stream->isVideo()) {
            sink.copyStream(input_stream);
        }
    }

    /* create decoder */
    fpp::DecoderContext video_decoder {
        source.stream(fpp::MediaType::Video)->params
    };

    /* encoder's options */
    fpp::Options video_options {
        { "threads",        "1"          }
        , { "thread_type",  "slice"       }
        , { "preset",       "ultrafast"   }
        , { "crf",          "30"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };

    /* create encoder */
    fpp::EncoderContext video_encoder {
        sink.stream(fpp::MediaType::Video)->params, video_options
    };

    const auto drow_text {
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
    fpp::VideoFilterContext video_filter {
        source.stream(fpp::MediaType::Video)->params
        , drow_text
    };

    /* open sink */
    sink.open();

    /* set timeout (because of endless rtsp stream) */
    const auto one_minute { 1 * 60 * 1000 };
    source.stream(fpp::MediaType::Video)->setEndTimePoint(one_minute);

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_packet {
        [&input_packet,&source]() {
            input_packet = source.read();
            return !input_packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        if (input_packet.isVideo()) {
            for (const auto& v_frame  : video_decoder.decode(input_packet)) {
            for (const auto& f_frame  : video_filter.filter(v_frame))       {
            for (const auto& v_packet : video_encoder.encode(f_frame))      {
                sink.write(v_packet);
            }}}
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
