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

/* USB video
video=HP Wide Vision FHD Camera
video=HP Wide Vision HD
video=Webcam C170
video=USB2.0 PC CAMERA
*/

/* USB audio
audio=Набор микрофонов (Realtek High Definition Audio)
*/

/* RTSP
rtsp://admin:admin@192.168.10.189:554/ch01.264
rtsp://admin:Admin2019@192.168.10.12:554
rtsp://admin:admin@192.168.10.3:554 (1080p + audio)
*/

void transmuxing_file() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    source.open();

    /* create sink */
    fpp::OutputFormatContext sink {
        "copy.flv"
    };

    /* copy source's streams to sink */
    for (const auto& input_stream : source.streams()) {
        sink.copyStream(input_stream);
    }

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
        sink.write(input_packet);
    }

    /* explicitly close contexts */
    source.close();
    sink.close();

}

void youtube_stream() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    source.open();

    /* create sink */
    const std::string stream_key {
        "aaaa-bbbb-cccc-dddd"
    };
    fpp::OutputFormatContext youtube {
        "rtmp://a.rtmp.youtube.com/live2/"
        + stream_key
    };

    /* copy source's streams to sink */
    for (const auto& input_stream : source.streams()) {
        if (input_stream->isVideo()) {
            youtube.copyStream( /* with predefined params */
                input_stream
                , fpp::utils::make_youtube_video_params()
            );
        }
        else if (input_stream->isAudio()) {
            youtube.copyStream( /* with predefined params */
                input_stream
                , fpp::utils::make_youtube_audio_params()
            );
        }
    }

    /* create decoders */
    fpp::DecoderContext video_decoder {
        source.stream(fpp::MediaType::Video)
    };
    fpp::DecoderContext audio_decoder {
        source.stream(fpp::MediaType::Audio)
    };

    /* create encoder's options */
    fpp::Options video_options {
        { "threads",        "1"          }
        , { "thread_type",  "slice"       }
        , { "preset",       "ultrafast"   }
        , { "crf",          "30"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };

    fpp::Options audio_options {
        { "preset", "low" }
    };

    /* create encoders */
    fpp::EncoderContext video_encoder {
        youtube.stream(fpp::MediaType::Video), video_options
    };
    fpp::EncoderContext audio_encoder {
        youtube.stream(fpp::MediaType::Audio), audio_options
    };

    /* create resampler */
    fpp::ResampleContext resample {{
        source.stream(fpp::MediaType::Audio)->params
        , youtube.stream(fpp::MediaType::Audio)->params
    }};

    /* open sink */
    youtube.open();

    const auto video_transcoding_required {
        fpp::utils::transcoding_required({
            source.stream(fpp::MediaType::Video)->params
            , youtube.stream(fpp::MediaType::Video)->params
        })
    };

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
            if (video_transcoding_required) {
                for (const auto& v_frame  : video_decoder.decode(input_packet)) {
                for (const auto& v_packet : video_encoder.encode(v_frame))      {
                    youtube.write(v_packet);
                }}
            }
            else {
                youtube.write(input_packet);
            }
        }
        else if (input_packet.isAudio()) {
            for (const auto& a_frame  : audio_decoder.decode(input_packet)) {
            for (const auto& ra_frame : resample.resample(a_frame))         {
            for (const auto& a_packet : audio_encoder.encode(ra_frame))     {
                youtube.write(a_packet);
            }}}
        }
    }

    /* explicitly close contexts */
    source.close();
    youtube.close();

}

void rtp_video_and_audio_stream() {

    const std::string ip {
        "127.0.0.1"
    };

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    source.open();

    /* create video sink */
    const auto rtp_port_video  { 16700 };
    const auto rtcp_port_video { rtp_port_video + 1 };
    fpp::OutputFormatContext rtp_video_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port_video)
            + "?rtcpport=" + std::to_string(rtcp_port_video)
    };

    /* copy only video stream to 1st sink */
    rtp_video_restreamer.copyStream(source.stream(fpp::MediaType::Video));

    /* create audio sink */
    const auto rtp_port_audio  { rtp_port_video + 2 };
    const auto rtcp_port_audio { rtp_port_audio + 1 };
    fpp::OutputFormatContext rtp_audio_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port_audio)
            + "?rtcpport=" + std::to_string(rtcp_port_audio)
    };

    /* copy only audio stream to 2nd sink */
    rtp_audio_restreamer.copyStream(source.stream(fpp::MediaType::Audio));
    /* fix audio stream index
     * (so that the player can distinguish video and audio stream) */
    rtp_audio_restreamer.stream(fpp::MediaType::Audio)->setIndex(1);

    /* open sinks */
    rtp_video_restreamer.open();
    rtp_audio_restreamer.open();

    /* create sdp file */
    const auto sdp {
        fpp::utils::merge_sdp_files(
            rtp_video_restreamer.sdp()
            , rtp_audio_restreamer.sdp()
        )
    };
    std::ofstream sdp_file;
    sdp_file.open("video_and_audio.sdp");
    sdp_file << sdp;
    sdp_file.close();

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
            rtp_video_restreamer.write(input_packet);
        }
        else if (input_packet.isAudio()) {
            input_packet.setStreamIndex(0);
            rtp_audio_restreamer.write(input_packet);
        }
    }

    /* explicitly close contexts */
    source.close();
    rtp_video_restreamer.close();
    rtp_audio_restreamer.close();

}

void rtp_video_stream() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    source.open();

    /* create sink */
    const std::string ip { "127.0.0.1" };
    const auto rtp_port  { 16700 };
    const auto rtcp_port { rtp_port + 1 };
    fpp::OutputFormatContext rtp_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port)
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* copy only video stream to sink */
    rtp_restreamer.copyStream(source.stream(fpp::MediaType::Video));

    /* open sink */
    rtp_restreamer.open();

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("video.sdp");
    sdp_file << rtp_restreamer.sdp();
    sdp_file.close();

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_video_packet {
        [&input_packet,&source]() {
            do {
                input_packet = source.read();
            } while (!input_packet.isVideo() && !input_packet.isEOF());
            return !input_packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_video_packet()) {
        rtp_restreamer.write(input_packet);
    }

    /* explicitly close contexts */
    source.close();
    rtp_restreamer.close();

}

void rtp_audio_stream() {

    /* create source */
    fpp::InputFormatContext source {
        "rtsp://admin:admin@192.168.10.3:554"
    };

    /* open source */
    source.open();

    /* create sink */
    const std::string ip { "127.0.0.1" };
    const auto rtp_port  { 16700 };
    const auto rtcp_port { rtp_port + 1 };
    fpp::OutputFormatContext rtp_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port)
            + "?rtcpport=" + std::to_string(rtcp_port)
    };

    /* copy only audio stream to sink */
    rtp_restreamer.copyStream(source.stream(fpp::MediaType::Audio));

    /* open sink */
    rtp_restreamer.open();

    /* create sdp file */
    std::ofstream sdp_file;
    sdp_file.open("audio.sdp");
    sdp_file << rtp_restreamer.sdp();
    sdp_file.close();

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_audio_packet {
        [&input_packet,&source]() {
            do {
                input_packet = source.read();
            } while (!input_packet.isAudio() && !input_packet.isEOF());
            return !input_packet.isEOF();
        }
    };

    /* read and write packet */
    while (read_audio_packet()) {
        /* fix audio packet's stream index */
        input_packet.setStreamIndex(0);
        rtp_restreamer.write(input_packet);
    }

    /* explicitly close contexts */
    source.close();
    rtp_restreamer.close();

}

auto main() -> int {

    std::cout << "Started\n";

    try {

//        transmuxing_file();
//        youtube_stream();
//        rtp_video_stream();
//        rtp_audio_stream();
//        rtp_video_and_audio_stream();

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
