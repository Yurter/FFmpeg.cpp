#include <iostream>
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/ResamplerContext.hpp>
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

#define USE_AUDIO

#include <fpp/core/time/Chronometer.hpp>

int main() {

    try {
        #pragma warning( push )
        #pragma warning( disable : 4974)
        ::av_register_all();
        #pragma warning( pop )
        ::avformat_network_init();
        ::avdevice_register_all();
//        ::av_log_set_level(AV_LOG_DEBUG);

        fpp::InputFormatContext camera { "rtsp://admin:admin@192.168.10.3:554" };
//        fpp::InputFormatContext camera { "findme.flv" };
        camera.open();
        std::cout << "Input context opened\n";

        const std::string stream_key { "apaj-8d8z-0ksj-6qxe" };
        fpp::OutputFormatContext youtube { "rtmp://a.rtmp.youtube.com/live2/" + stream_key };
//        fpp::OutputFormatContext youtube { "findme2.flv" };
        for (const auto& stream : camera.streams()) {
            const auto input_params { stream->params };
            if (input_params->isVideo()) {
                const auto output_params { fpp::utils::make_youtube_video_params() };
                output_params->completeFrom(input_params);
                youtube.createStream(output_params);
            }
#ifdef USE_AUDIO
            if (input_params->isAudio()) {
                const auto output_params { fpp::utils::make_youtube_audio_params() };
                output_params->completeFrom(input_params);
                youtube.createStream(output_params);
            }
#endif
        }
        youtube.open();
        ::av_dump_format(youtube.raw(), 0, youtube.mediaResourceLocator().c_str(), 1);
        std::cout << "Output context opened\n";

        fpp::DecoderContext video_decoder { camera.stream(0) };
#ifdef USE_AUDIO
        fpp::DecoderContext audio_decoder { camera.stream(1) };
#endif
        std::cout << "Decoders inited\n";

#ifdef USE_AUDIO
        fpp::ResamplerContext resampler {
            { camera.stream(1)->params
            , youtube.stream(1)->params }
        };
        std::cout << "Resampler inited\n";
#endif

        fpp::Dictionary options;
//        options.setOption("threads",        "1"         );
//        options.setOption("thread_type",    "slice"     );
        options.setOption("preset",         "fast" );
        options.setOption("crf",            "30"        );
        options.setOption("tune",           "zerolatency");
        options.setOption("profile",        "main");
        fpp::EncoderContext video_encoder { youtube.stream(0), std::move(options) };

        auto out_stream { youtube.stream(0)->raw() };
        out_stream->codecpar->extradata_size = video_encoder.raw()->extradata_size;
        out_stream->codecpar->extradata = static_cast<uint8_t*>(av_mallocz(video_encoder.raw()->extradata_size));
        memcpy(out_stream->codecpar->extradata, video_encoder.raw()->extradata, video_encoder.raw()->extradata_size);



#ifdef USE_AUDIO
        fpp::Dictionary audio_options;
        audio_options.setOption("preset",   "low" );
        fpp::EncoderContext audio_encoder { youtube.stream(1), std::move(audio_options) };
        auto out_stream_audio { youtube.stream(1)->raw() };
        youtube.stream(0)->setStampType(fpp::StampType::Realtime);
        out_stream_audio->codecpar->extradata_size = audio_encoder.raw()->extradata_size;
        out_stream_audio->codecpar->extradata = static_cast<uint8_t*>(av_mallocz(audio_encoder.raw()->extradata_size));
        memcpy(out_stream_audio->codecpar->extradata, audio_encoder.raw()->extradata, audio_encoder.raw()->extradata_size);
#endif
        std::cout << "Encoders inited\n";

        if (avformat_write_header(youtube.raw(), nullptr) < 0)
        {
            fprintf(stderr, "Could not write header!\n");
            return 1;
        }

        const auto transcode_video { false };

        fpp::Chronometer chronometer;

        while (true) {
            static int timeout = 0;
            timeout++;
            if (timeout == 500) {
//                break;
            }
            if (chronometer.elapsed_milliseconds() > (3600000 / 2)) {
                break;
            }
            /*const */auto input_packet { camera.read() };

            if (input_packet.isVideo()) {
//                std::cout << input_packet.toString() << "\n";
                if (transcode_video) {
//                    ::av_log_set_level(AV_LOG_INFO);
                    const auto video_frames { video_decoder.decode(input_packet) };
                    for (const auto& v_frame : video_frames) {
//                        ::av_log_set_level(AV_LOG_DEBUG);
//                        std::cout << "v_frame: " << v_frame.raw().pict_type = AV_PICTURE_TYPE_NONE << "\n";
                        const auto video_packets { video_encoder.encode(v_frame) };
//                        ::av_log_set_level(AV_LOG_INFO);
                        for (const auto& v_packet : video_packets) {
                            auto v_packet_copy { v_packet };
//                            std::cout << v_packet_copy.toString() << "\n";
                            youtube.write(v_packet);
                        }
                    }
                }
                else {
                    youtube.write(input_packet);
                }
            }
#ifdef USE_AUDIO
            else if (input_packet.isAudio()) {
                input_packet.setDts(AV_NOPTS_VALUE);
                input_packet.setPts(AV_NOPTS_VALUE);
//                std::cout << "[1] " << input_packet.toString() << "\n";
                const auto audio_frames { audio_decoder.decode(input_packet) };
                for (const auto& a_frame : audio_frames) {
                    const auto resampled_frames { resampler.resample(a_frame) };
                    for (auto& ra_frame : resampled_frames) {
                        auto ra_frame_copy { ra_frame };
                        static auto prev_pts { 0ll };
//                        if (prev_pts == ra_frame_copy.pts()) {
//                            ra_frame_copy.setPts(ra_frame_copy.pts() + 1);
//                        }
                        prev_pts = ra_frame_copy.pts();
//                        std::cout << "[ra] " << ra_frame_copy << "\n";
                        auto audio_packets { audio_encoder.encode(ra_frame_copy) };
                        for (auto& a_packet : audio_packets) {
                            static int temp_counter = 0;
//                            a_packet.setDts(temp_counter);
//                            a_packet.setPts(temp_counter);
                            temp_counter++;
//                            std::cout << a_packet.toString() << "\n";
                            youtube.write(a_packet);
                        }
                    }
                }
            }
            else {
                std::cout << "Unknown packet!\n";
                break;
            }
#endif
        }

    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception\n";
    }

    std::cout << "Finished\n";

    return 0;

}
