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

/*

======================== USB video ========================

    video=HP Wide Vision FHD Camera
    video=HP Wide Vision HD
    video=Webcam C170
    video=USB2.0 PC CAMERA
    video=KVYcam Video Driver

======================== USB audio ========================

    audio=Набор микрофонов (Realtek High Definition Audio)

========================== RTSP ===========================

private:

    rtsp://admin:admin@192.168.10.189:554/ch01.264
    rtsp://admin:Admin2019@192.168.10.12:554
    rtsp://admin:admin@192.168.10.3:554 (1080p + audio)

public:

    (!)Check channels ch01_0, ch02_0 and ch03_0
    for lower quality but lower latency

    rtsp://80.26.155.227/live/ch00_0    (food store)
    rtsp://195.46.114.132/live/ch00_0   (building)
    rtsp://87.197.138.187/live/ch00_0   (sports nutrition store (with audio))
    rtsp://213.129.131.54/live/ch00_0   (parking place)
    rtsp://90.80.246.160/live/ch00_0    (lobby)
    rtsp://186.38.89.5/live/ch00_0      (tree)
    rtsp://82.79.117.37/live/ch00_0     (auto parts store)
    rtsp://75.147.239.197/live/ch00_0   (street)
    rtsp://109.183.182.53/live/ch00_0   (yard)
    rtsp://205.120.142.79/live/ch00_0   (server room)
    rtsp://79.101.6.26/live/ch00_0      (tennis tables)
    rtsp://186.1.213.236/live/ch00_0    (street)
    rtsp://185.41.129.79/live/ch00_0    (aquapark)
    rtsp://109.70.190.112/live/ch00_0   (street)
    rtsp://109.73.212.169/live/ch00_0   (turtles)
    rtsp://91.197.91.139/live/ch00_0    (park)
    rtsp://82.150.185.64/live/ch00_0    (field)
    rtsp://80.76.108.241/live/ch00_0    (yard)
    rtsp://193.124.147.207/live/ch00_0  (tree)
    rtsp://163.47.188.104/live/ch00_0
    rtsp://98.163.61.242/live/ch00_0
    rtsp://98.163.61.243/live/ch00_0
    rtsp://38.130.64.34/live/ch00_0

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
        source.stream(fpp::MediaType::Video)->params
    };
    fpp::DecoderContext audio_decoder {
        source.stream(fpp::MediaType::Audio)->params
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
        youtube.stream(fpp::MediaType::Video)->params, video_options
    };
    fpp::EncoderContext audio_encoder {
        youtube.stream(fpp::MediaType::Audio)->params, audio_options
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
//    Important notes:
//
//    If rtcpport is not set the RTCP port will be set to the RTP port value plus 1.
//    If localrtpport (the local RTP port) is not set any available port will be used for the local RTP and RTCP ports.
//    If localrtcpport (the local RTCP port) is not set it will be set to the local RTP port value plus 1.
    fpp::OutputFormatContext rtp_restreamer {
        "rtp://" + ip + ":" + std::to_string(rtp_port) // TODO add local optional 31.03 (localrtpport, localrtcpport)
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

void webcam_to_file() {

    /* create source */
    fpp::InputFormatContext webcam {
        "video=HP Wide Vision HD"
    };

    /* change default image size (480x360) */
    fpp::Options webcam_options {
        { "video_size", "1280x720" }
    };

    /* open source */
    webcam.open(webcam_options);

    /* create sink */
    fpp::OutputFormatContext video_file {
        "webcam.flv"
    };

    /* encode video because of camera's rawvideo codec */
    const auto out_params { fpp::VideoParameters::make_shared() };
    out_params->setEncoder(AVCodecID::AV_CODEC_ID_H264);
    out_params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV420P);
    out_params->setGopSize(12);

    /* copy source's video stream to sink */
    for (const auto& input_stream : webcam.streams()) {
        if (input_stream->isVideo()) {
            video_file.copyStream( /* with predefined params */
                input_stream
                , out_params
            );
        }
    }

    /* create decoder */
    fpp::DecoderContext video_decoder {
        webcam.stream(fpp::MediaType::Video)->params
    };

    /* create encoder's options */
    fpp::Options video_options {
        { "threads",        "1"          }
        , { "thread_type",  "slice"       }
        , { "preset",       "ultrafast"   }
        , { "crf",          "15"          } // 0-51
        , { "profile",      "main"        }
        , { "tune",         "zerolatency" }
    };

    /* create encoders */
    fpp::EncoderContext video_encoder {
        video_file.stream(fpp::MediaType::Video)->params, video_options
    };

    /* create rescaler (because of pixel format mismatch) */
    fpp::RescaleContext rescaler {{
        webcam.stream(fpp::MediaType::Video)->params
        , video_file.stream(fpp::MediaType::Video)->params
    }};

    /* open sink */
    video_file.open();

    fpp::Packet input_packet {
        fpp::MediaType::Unknown
    };
    const auto read_packet {
        [&input_packet,&webcam]() {
            input_packet = webcam.read();
            return !input_packet.isEOF();
        }
    };

    /* because of endless webcam's video */
    webcam.stream(0)->setEndTimePoint(10 * 1000);

    /* read and write packets */
    while (read_packet()) {
        if (input_packet.isVideo()) {
            for (const auto& v_frame  : video_decoder.decode(input_packet)) {
            const auto rv_frame { rescaler.scale(v_frame) };
            for (const auto& v_packet : video_encoder.encode(rv_frame))     {
                video_file.write(v_packet);
            }}
        }
    }

    /* explicitly close contexts */
    webcam.close();
    video_file.close();

}

auto main() -> int {

    std::cout << "Started\n";

    try {

//        transmuxing_file();
//        youtube_stream();
//        rtp_video_stream();
//        rtp_audio_stream();
//        rtp_video_and_audio_stream();
//        text_on_video();
//        webcam_to_file();

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
