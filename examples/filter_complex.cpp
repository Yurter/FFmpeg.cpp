#include "examples.hpp"
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/filter/ComplexFilterGraph.hpp>
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

    /* create output params based on 1st source's params */
    const auto inpar {
        sources[0].stream(fpp::MediaType::Audio)->params
    };
    const auto outpar {
        fpp::AudioParameters::make_shared()
    };
    outpar->completeFrom(inpar);

    /* create codecs */
    std::array<fpp::DecoderContext,N> decoders {
          fpp::DecoderContext { sources[0].stream(fpp::MediaType::Audio)->params }
        , fpp::DecoderContext { sources[1].stream(fpp::MediaType::Audio)->params }
        , fpp::DecoderContext { sources[2].stream(fpp::MediaType::Audio)->params }
    };
    fpp::EncoderContext encoder { outpar };

    /* create filter graph with 3 input chains and 1 output */
    const fpp::Options graph_opt {
        { "threads", "1" }
    };
    fpp::ComplexFilterGraph graph { graph_opt };
    const std::array<std::size_t, N> input_chain_index {
          graph.createInputFilterChain(sources[0].stream(fpp::MediaType::Audio)->params, { "adelay=1000" })
        , graph.createInputFilterChain(sources[1].stream(fpp::MediaType::Audio)->params, { "adelay=2000", "volume=1" })
        , graph.createInputFilterChain(sources[2].stream(fpp::MediaType::Audio)->params, { "volume=3" })
    };
    const auto output_chain_index {
        graph.createOutputFilterChain(outpar, { "amix=inputs=3:dropout_transition=0" })
    };
    graph.link({ input_chain_index[0], input_chain_index[1], input_chain_index[2] }, { output_chain_index });
    graph.init();

    /* create sink */
    fpp::OutputFormatContext sink {
        "mixed_audio.mp3"
    };

    /* create output stream */
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
                graph.write(a_frame, input_chain_index[i]);
                for (const auto& fa_frame : graph.read(output_chain_index)) {
                for (/*const*/ auto& a_packet : encoder.encode(fa_frame))   {
                    a_packet.setStreamIndex(0);
                    a_packet.setTimeBase(inpar->timeBase());
                    sink.write(a_packet);
                }}}
            }
            else {
                continue;
            }
        }
        if (all_empty) {
            break;
        }
    }

    /* explicitly close contexts */
    for (auto& src : sources) {
        src.close();
    }
    sink.close();

}
