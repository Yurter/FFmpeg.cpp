#include <iostream>
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/ResampleContext.hpp>
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavdevice/avdevice.h>
}

/*
video=HP Wide Vision FHD Camera
video=Webcam C170
video=USB2.0 PC CAMERA
rtsp://admin:admin@192.168.10.189:554/ch01.264
rtsp://admin:Admin2019@192.168.10.12:554
rtsp://admin:admin@192.168.10.3:554 (1080p)
*/

void simpleCopyFile() {

    /* create source */
    fpp::InputFormatContext camera { "rtsp://admin:admin@192.168.10.3:554" };

    /* create sink */
    fpp::OutputFormatContext youtube { "rtsp_copy.flv" };

    /* open source */
    camera.open();

    /* copy source's streams to sink */
    for (const auto& input_stream : camera.streams()) {
        youtube.copyStream(input_stream);
    }

    /* open sink */
    youtube.open();

    /* set timeout */
    camera.stream(fpp::MediaType::Video)->setEndTimePoint(1 * 60 * 1000); // 1 min

    fpp::Packet input_packet { fpp::MediaType::Unknown };
    const auto read_packet {
        [&input_packet,&camera]() {
            input_packet = camera.read();
            return !input_packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        youtube.write(input_packet);
    }

}

void startYoutubeStream() {

    /* create source */
    fpp::InputFormatContext camera { "rtsp://admin:admin@192.168.10.3:554" };

#define FPP_DEBUG

    /* create sink */
#ifndef FPP_DEBUG
    const std::string stream_key { "apaj-8d8z-0ksj-6qxe" };
    fpp::OutputFormatContext youtube {
        "rtmp://a.rtmp.youtube.com/live2/"
        + stream_key
    };
#endif
#ifdef FPP_DEBUG
    fpp::OutputFormatContext youtube { "youtube.flv" };
#endif

    /* open source */
    camera.open();

    /* copy source's streams to sink */
    for (const auto& input_stream : camera.streams()) {
        if (input_stream->isVideo()) {
            youtube.copyStream(
                input_stream, fpp::utils::make_youtube_video_params()
            );
        }
        if (input_stream->isAudio()) {
            youtube.copyStream(
                input_stream, fpp::utils::make_youtube_audio_params()
            );
        }
    }

    /* create decoders */
    fpp::DecoderContext video_decoder { camera.stream(fpp::MediaType::Video) };
    fpp::DecoderContext audio_decoder { camera.stream(fpp::MediaType::Audio) };

    /* create encoder's options */
    fpp::Dictionary video_options;
    video_options.setOption("threads",      "1"     );
    video_options.setOption("thread_type",  "slice" );
    video_options.setOption("preset",       "fast"  );
    video_options.setOption("crf",          "30"    );
    video_options.setOption("profile",      "main"  );
    video_options.setOption("tune",         "zerolatency");

    fpp::Dictionary audio_options;
    audio_options.setOption("preset", "low" );

    /* create encoders */
    fpp::EncoderContext video_encoder { youtube.stream(fpp::MediaType::Video), std::move(video_options) };
    fpp::EncoderContext audio_encoder { youtube.stream(fpp::MediaType::Audio), std::move(audio_options) };
    std::cout << "video_encoder " << video_encoder.toString() << std::endl;
    std::cout << "audio_encoder " << audio_encoder.toString() << std::endl;

    /* create resampler */
    fpp::ResampleContext resample {
        { camera.stream(fpp::MediaType::Audio)->params
        , youtube.stream(fpp::MediaType::Audio)->params }
    };

    /* open sink */
    youtube.open();

    const auto transcode_video {
        fpp::utils::transcoding_required(
            { camera.stream(fpp::MediaType::Video)->params
            , youtube.stream(fpp::MediaType::Video)->params })
    };

    /* set timeout */
    camera.stream(fpp::MediaType::Video)->setEndTimePoint(10 * 60 * 1000); // 10 min

    fpp::Packet input_packet { fpp::MediaType::Unknown };
    const auto read_packet {
        [&input_packet,&camera]() {
            input_packet = camera.read();
            return !input_packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_packet()) {
        if (input_packet.isVideo()) {
            if (transcode_video) {
                const auto video_frames { video_decoder.decode(input_packet) };
                for (const auto& v_frame : video_frames) {
                    const auto video_packets { video_encoder.encode(v_frame) };
                    for (const auto& v_packet : video_packets) {
                        youtube.write(v_packet);
                    }
                }
            }
            else {
                youtube.write(input_packet);
            }
        }
        else if (input_packet.isAudio()) {
            const auto audio_frames { audio_decoder.decode(input_packet) };
            for (const auto& a_frame : audio_frames) {
                const auto resampled_frames { resample.resample(a_frame) };
                for (auto& ra_frame : resampled_frames) {
                    auto audio_packets { audio_encoder.encode(ra_frame) };
                    for (auto& a_packet : audio_packets) {
                        youtube.write(a_packet);
                    }
                }
            }
        }
        else {
            throw std::runtime_error { "Unknown packet! "};
        }
    }

}

int main() {

    try {

        #pragma warning( push )
        #pragma warning( disable : 4974)
        ::av_register_all();
        #pragma warning( pop )
        ::avformat_network_init();
        ::avdevice_register_all();

        startYoutubeStream();
//        simpleCopyFile();

    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception\n";
    }

    std::cout << "Finished\n";

    return 0;

}
