#include <iostream>
#include <fstream>
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

    /* open source */
    camera.open();

    /* create sink */
    fpp::OutputFormatContext youtube { "rtsp_copy.flv" };

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
#define FPP_DEBUG

    /* create source */
    fpp::InputFormatContext camera { "rtsp://admin:admin@192.168.10.3:554" };

    /* open source */
    camera.open();

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
    fpp::Options video_options {
        { "threads",        "1"     }
        , { "thread_type",  "slice" }
        , { "preset",       "fast"  }
        , { "crf",          "30"    }
        , { "profile",      "main"  }
        , { "tune",         "zerolatency" }
    };

    fpp::Options audio_options {
        { "preset", "low" }
    };

    /* create encoders */
    fpp::EncoderContext video_encoder { youtube.stream(fpp::MediaType::Video), video_options };
    fpp::EncoderContext audio_encoder { youtube.stream(fpp::MediaType::Audio), audio_options };

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
#ifndef FPP_DEBUG
    camera.stream(fpp::MediaType::Video)->setEndTimePoint(10 * 60 * 1000); // 10 min
#endif
#ifdef FPP_DEBUG
    camera.stream(fpp::MediaType::Video)->setEndTimePoint(20 * 1000); // 20 sec
#endif

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

#undef FPP_DEBUG
}

void rtp_video_and_audio_stream() {

    /* create source */
    fpp::InputFormatContext camera { "rtsp://admin:admin@192.168.10.3:554" };

    /* open source */
    camera.open();

    const std::string ip { "127.0.0.1" };

    /* create video sink */
    const auto rtp_port_video  { 700 };
    const auto rtcp_port_video { rtp_port_video + 1 };
    fpp::OutputFormatContext rtp_video_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port_video)
            + "?rtcpport=" + std::to_string(rtcp_port_video)
    };

    /* copy only video stream to 1st sink */
    rtp_video_restreamer.copyStream(camera.stream(fpp::MediaType::Video));

    /* create audio sink */
    const auto rtp_port_audio  { rtp_port_video + 2 };
    const auto rtcp_port_audio { rtp_port_audio + 1 };
    fpp::OutputFormatContext rtp_audio_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port_audio)
            + "?rtcpport=" + std::to_string(rtcp_port_audio)
    };

    /* copy only audio stream to 2nd sink */
    rtp_audio_restreamer.copyStream(camera.stream(fpp::MediaType::Audio));
    /* fix  */
    rtp_audio_restreamer.stream(fpp::MediaType::Audio)->setIndex(1);

    /* open sinks */
    rtp_video_restreamer.open();
    rtp_audio_restreamer.open();

    const auto sdp {
        fpp::utils::merge_sdp_files(
            rtp_video_restreamer.sdp()
            , rtp_audio_restreamer.sdp()
        )
    };

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("video_and_audio.sdp");
    sdp_file << sdp;
    sdp_file.close();

    fpp::Packet input_packet { fpp::MediaType::Unknown };
    const auto read_video_packet {
        [&input_packet,&camera]() {
            input_packet = camera.read();
            return !input_packet.isEOF();
        }
    };

    /* read and write packets */
    while (read_video_packet()) {
        if (input_packet.isVideo()) {
            rtp_video_restreamer.write(input_packet);
        }
        else if (input_packet.isAudio()) {
            input_packet.setStreamIndex(0);
            rtp_audio_restreamer.write(input_packet);
        }
    }

}

void rtp_video_stream() {

    /* create source */
    fpp::InputFormatContext camera {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    camera.open();

    /* create sink */
    const std::string ip { "127.0.0.1" };
    const auto rtp_port  { 700 };
    const auto rtcp_port { rtp_port + 1 };
    fpp::OutputFormatContext rtp_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port)
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* copy only video stream to sink */
    rtp_restreamer.copyStream(camera.stream(fpp::MediaType::Video));

    /* open sink */
    rtp_restreamer.open();

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("video.sdp");
    sdp_file << rtp_restreamer.sdp();
    sdp_file.close();

    fpp::Packet input_packet { fpp::MediaType::Unknown };
    const auto read_video_packet {
        [&input_packet,&camera]() {
            do {
                input_packet = camera.read();
            } while (!input_packet.isVideo() && !input_packet.isEOF());
            return !input_packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_video_packet()) {
        rtp_restreamer.write(input_packet);
    }

}

void rtp_audio_stream() {

    /* create source */
    fpp::InputFormatContext camera {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    camera.open();

    /* create sink */
    const std::string ip { "127.0.0.1" };
    const auto rtp_port  { 700 };
    const auto rtcp_port { rtp_port + 1 };
    fpp::OutputFormatContext rtp_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port)
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* copy only audio stream to sink */
    rtp_restreamer.copyStream(camera.stream(fpp::MediaType::Audio));

    /* open sink */
    rtp_restreamer.open();

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("audio.sdp");
    sdp_file << rtp_restreamer.sdp();
    sdp_file.close();

    fpp::Packet input_packet { fpp::MediaType::Unknown };
    const auto read_audio_packet {
        [&input_packet,&camera]() {
            do {
                input_packet = camera.read();
            } while (!input_packet.isAudio() && !input_packet.isEOF());
            return !input_packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_audio_packet()) {
        input_packet.setStreamIndex(0);
        rtp_restreamer.write(input_packet);
    }

}

int main() {

    try {

//        #pragma warning( push )
//        #pragma warning( disable : 4974)
        ::av_register_all();
//        #pragma warning( pop )
        ::avformat_network_init();
        ::avdevice_register_all();

//        startYoutubeStream();
//        simpleCopyFile();
        rtp_video_and_audio_stream();
//        rtp_video_stream();
//        rtp_audio_stream();

    } catch (const fpp::FFmpegException& e) {
        std::cout << "FFmpegException: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception\n";
    }

    std::cout << "Finished\n";

    return 0;

}
