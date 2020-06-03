#include "examples.hpp"
#include <fpp/format/InputFormatContext.hpp>
#include <fpp/format/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/resample/ResampleContext.hpp>

void mic_to_file() {

    /* create source */
    fpp::InputFormatContext source {
        "audio=Microphone (2- Webcam C170)"
    };

    /* open source */
    if (!source.open()) {
        return;
    }

    /* create sink */
    fpp::OutputFormatContext sink {
        "mic.aac"
    };

    /* create stream with predefined params */
    const auto in_params  { source.stream(fpp::MediaType::Audio)->params };
    const auto out_params { fpp::AudioParameters::make_shared() };
    out_params->setEncoder(AVCodecID::AV_CODEC_ID_AAC);
    out_params->setSampleFormat(AV_SAMPLE_FMT_FLTP);
    out_params->setTimeBase(DEFAULT_TIME_BASE);
    out_params->setSampleRate(44'100);
    out_params->setBitrate(128 * 1024);
    out_params->setChannelLayout(AV_CH_LAYOUT_STEREO);
    out_params->setChannels(2);
    out_params->completeFrom(in_params);

    sink.createStream(out_params);

    /* create decoder */
    fpp::DecoderContext audio_decoder {
        source.stream(fpp::MediaType::Audio)->params
    };

    /* create encoder */
    fpp::EncoderContext audio_encoder {
        sink.stream(fpp::MediaType::Video)->params
    };

    /* create rescaler (because of pixel format mismatch) */
    fpp::ResampleContext resampler {{
        source.stream(fpp::MediaType::Audio)->params
        , sink.stream(fpp::MediaType::Audio)->params
    }};

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

    /* because of endless webcam's video */
    sink.stream(0)->setEndTimePoint(10 * 1000);

    auto stop_flag { false };

    /* read and write packets */
    while (read_packet() && !stop_flag) {
        if (packet.isAudio()) {
            for (const auto& a_frame  : audio_decoder.decode(packet))   {
            for (const auto& ra_frame : resampler.resample(a_frame))    {
            for (      auto& a_packet : audio_encoder.encode(ra_frame)) {
                stop_flag = !sink.write(a_packet);
            }}}
        }
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}
