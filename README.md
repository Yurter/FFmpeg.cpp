# FFmpeg.cpp
FFmpeg.cpp is a C++ wrapper for FFmpeg libraries
## How To Use
#### Read from source
    fpp::InputFormatContext source {
        "rtsp://205.120.142.79/live/ch00_0"
    };
    if (!source.open()) {
        return;
    }
    fpp::Packet packet {
        source.read();
    };
#### Write to sink
    fpp::OutputFormatContext sink {
        "filename.flv"
    };
    sink.copyStream(source.stream(fpp::Media::Type::Video));
    if (!sink.open()) {
        return;
    }
    sink.write(packet);
#### Decoding
    fpp::DecoderContext video_decoder {
        source.stream(fpp::Media::Type::Video)->params
    };
    for (const auto& frame : video_decoder.decode(packet)) {
        ...
    }
#### Encoding
    fpp::EncoderContext video_encoder {
        sink.stream(fpp::Media::Type::Video)->params
    };
    for (const auto& packet : video_encoder.encode(frame)) {
        ...
    }
#### Rescaling
    fpp::RescaleContext rescaler {{
        source.stream(fpp::Media::Type::Video)->params
        , sink.stream(fpp::Media::Type::Video)->params
    }};
    const auto r_frame { rescaler.scale(frame) };
#### Resampling
    fpp::ResampleContext resample {{
        source.stream(fpp::Media::Type::Audio)->params
        , sink.stream(fpp::Media::Type::Audio)->params
    }};
    for (const auto& r_frame : resample.resample(frame)) {
        ...
    }
#### Linear filter
    // Timelapse filter (speed x3):
    const std::vector<std::string> filters {
          "select='not(mod(n,3))'"
        , "setpts=0.333*PTS"
    };
    fpp::LinearFilterGraph filter_graph {
          source.stream(fpp::Media::Type::Video)->params
        , filters
    };
    for (const auto& f_frame : filter_graph.filter(v_frame)) {
        ...
    }
#### Complex filter
    fpp::ComplexFilterGraph graph;
    const std::array<std::size_t, 3> input_chain_index {
          graph.createInputFilterChain(in_audio_par[0], { "adelay=1000" })
        , graph.createInputFilterChain(in_audio_par[1], { "adelay=2000", "volume=1" })
        , graph.createInputFilterChain(in_audio_par[2], { "volume=3" })
    };
    const auto output_chain_index {
        graph.createOutputFilterChain(outpar, { "amix=inputs=3:dropout_transition=0" })
    };
    graph.link({ input_chain_index[0], input_chain_index[1], input_chain_index[2] }, { output_chain_index });
    graph.init();
    
    for (std::size_t i { 0 }; i < sources.size(); ++i) {
        ...
        graph.write(frame, input_chain_index[i]);
        ...
        for (const auto& f_frame : graph.read(output_chain_index)) {
            ...
        }
    }
## Examples
To see more: transcoding, screen capture, webcam recording, rtp stream, youtube stream, etc., check the [examples](https://github.com/Yurter/FFmpeg.cpp/tree/master/examples)
