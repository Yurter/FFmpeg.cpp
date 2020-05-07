#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/ResampleContext.hpp>
#include <fpp/refi/FilterGraph.hpp>
#include <array>

void complex() {

    /* create sources */
    constexpr auto N { 3 };
    std::array<fpp::InputFormatContext,N> sources {
          fpp::InputFormatContext { "audio_0.mp3" }
        , fpp::InputFormatContext { "audio_1.mp3" }
        , fpp::InputFormatContext { "audio_2.mp3" }
    };

    /* open sources */
    for (auto& src : sources) {
        if (!src.open()) {
            return;
        }
    }

    /* create output stream with input params */
    const auto inpar {
        sources[0].stream(fpp::MediaType::Audio)->params
    };
    const auto outpar {
        fpp::AudioParameters::make_shared()
    };
    outpar->completeFrom(inpar);

    /* create codecs */
    std::array<fpp::DecoderContext,N> decoders {
          sources[0].stream(fpp::MediaType::Audio)->params
        , sources[1].stream(fpp::MediaType::Audio)->params
        , sources[2].stream(fpp::MediaType::Audio)->params
    };
    fpp::EncoderContext video_encoder { outpar };

    /* create filter graph */
    fpp::FilterGraph graph;

    const std::array<std::size_t, N> input_chain {
          graph.createInputFilterChain(sources[0].stream(fpp::MediaType::Audio)->params, { "adelay=1000" })
        , graph.createInputFilterChain(sources[1].stream(fpp::MediaType::Audio)->params, { "adelay=2000", "volume=1" })
        , graph.createInputFilterChain(sources[2].stream(fpp::MediaType::Audio)->params, { "adelay=0|0",  "volume=3" })
    };

    const auto output_chain { graph.createOutputFilterChain(outpar, { "amix=inputs=3:dropout_transition=0" })};

    graph.link({ input_chain[0], input_chain[1], input_chain[2] }, { output_chain });

    /* create sink */
    fpp::OutputFormatContext sink {
        "mixed_audio.mp3"
    };

    sink.createStream(outpar);

    /* open sink */
    if (!sink.open()) {
        return;
    }

    /* read and write packet */
    while (true) {

        auto all_empty { true };
        for (std::size_t i { 0 }; i < sources.size(); ++i) {
            const auto packet { sources[i].read() };
            if (!packet.isEOF()) {
                all_empty = false;

                for (const auto& a_frame  : decoders[i].decode(packet))     {
                graph.write(a_frame, input_chain[i]);
                for (const auto& fa_frame : graph.read(output_chain))       {
                for (const auto& a_packet : video_encoder.encode(fa_frame)) {
                    sink.write(a_packet);
                }}}
            }
            else {
                continue;
            }
        }
    }

    /* explicitly close contexts */
    for (auto& src : sources) {
        src.close();
    }
    sink.close();

}
