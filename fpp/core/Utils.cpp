#include "Utils.hpp"
#include <thread>
#include <atomic>
#include <algorithm>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/stream/AudioParameters.hpp>

extern "C" {
    #include <libavutil/imgutils.h>
}

#define both_params_is_video(params) \
    do {\
        if_not(params.in->isVideo()) {\
            throw std::runtime_error {\
                std::string { __FUNCTION__ } + " failed - in is not a video param"\
            };\
        }\
        if_not(params.out->isVideo()) {\
            throw std::runtime_error {\
                std::string { __FUNCTION__ } + " failed - out is not a video param"\
            };\
        }\
    } while (false)

#define both_params_is_audio(params) \
    do {\
        if_not(params.in->isAudio()) {\
            throw std::runtime_error {\
                std::string { __FUNCTION__ } + " failed - in is not a audio param"\
            };\
        }\
        if_not(params.out->isAudio()) {\
            throw std::runtime_error {\
                std::string { __FUNCTION__ } + " failed - out is not a audio param"\
            };\
        }\
    } while (false)

#define params_has_same_type(params) \
    do {\
        if (params.in->type() != params.out->type()) {\
            throw std::runtime_error {\
                std::string { __FUNCTION__ }\
                    + " failed - in is " + to_string(params.in->type())\
                    + " but out is " + to_string(params.out->type())\
            };\
        }\
    } while (false)

namespace fpp {

    std::string utils::to_string(MediaType type) {
        switch (type) {
        case MediaType::Unknown:
            return "Unknown";
        case MediaType::Video:
            return "Video";
        case MediaType::Audio:
            return "Audio";
        case MediaType::EndOF:
            return "EOF";
        }
        return "Invalid";
    }

    std::string utils::pts_to_string(int64_t pts) {
        return pts == AV_NOPTS_VALUE ? "NOPTS" : std::to_string(pts);
    }

    std::string utils::to_string(bool value) {
        return value ? "true" : "false";
    }

    std::string utils::to_string(AVPixelFormat pxl_fmt) {
        const char* ret { ::av_get_pix_fmt_name(pxl_fmt) };
        if (not_inited_ptr(ret)) {
            return "NONE";
        }
        return std::string(ret);
    }

    std::string utils::to_string(AVSampleFormat value) {
        const char* ret { ::av_get_sample_fmt_name(value) };
        if (not_inited_ptr(ret)) {
            return "NONE";
        }
        return std::string(ret);
    }

    void utils::sleep_for(int64_t milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void utils::sleep_for_ms(int64_t milliseconds) {
        sleep_for(milliseconds);
    }

    void utils::sleep_for_sec(int64_t seconds) {
        sleep_for_ms(seconds * 1'000);
    }

    void utils::sleep_for_min(int64_t minutes) {
        sleep_for_sec(minutes * 60);
    }

    std::string utils::time_to_string(int64_t time_stamp, AVRational time_base) {
        const auto time_ms = av_rescale_q(time_stamp, time_base, DEFAULT_TIME_BASE);
        const int64_t ms = time_ms % 1000;
        const int64_t ss = (time_ms / 1000) % 60;
        const int64_t mm = ((time_ms / 1000) % 3600) / 60;
        const int64_t hh = (time_ms / 1000) / 3600;
        return std::to_string(hh) + ':' + std::to_string(mm) + ':' + std::to_string(ss) + '.' + std::to_string(ms);
    }

    bool utils::exit_code(Code code) {
        if (error_code(code))                   { return true; }
        if (code == Code::EXIT)                 { return true; }
        if (code == Code::END_OF_FILE)          { return true; }
        return false;
    }

    bool utils::error_code(Code code) {
        if (code == Code::ERR)                  { return true; }
        if (code == Code::EXCEPTION)            { return true; }
        if (code == Code::NOT_INITED)           { return true; }
        if (code == Code::FFMPEG_ERROR)         { return true; }
        if (code == Code::INVALID_INPUT)        { return true; }
        if (code == Code::NOT_IMPLEMENTED)      { return true; }
        if (code == Code::INVALID_CALL_ORDER)   { return true; }
        return false;
    }

    std::string utils::to_string(Code code) {
        if (code == Code::OK)                   { return "OK";                      }
        if (code == Code::ERR)                  { return "Error";                   }
        if (code == Code::EXIT)                 { return "Exit";                    }
        if (code == Code::AGAIN)                { return "Again";                   }
        if (code == Code::NOT_INITED)           { return "Not inited";              }
        if (code == Code::END_OF_FILE)          { return "EOF";                     }
        if (code == Code::FFMPEG_ERROR)         { return "FFmpeg error";            }
        if (code == Code::INVALID_INPUT)        { return "Invalid input";           }
        if (code == Code::NOT_IMPLEMENTED)      { return "Method not implemented";  }
        if (code == Code::INVALID_CALL_ORDER)   { return "Invalid call order";      }
        return "Unknown error code: " + std::to_string(int(code));
    }

    std::string utils::to_string(AVRational rational) {
        return std::to_string(rational.num)
                + "/" +
                std::to_string(rational.den);
    }

    bool utils::compatible_with_pixel_format(const AVCodec* codec, AVPixelFormat pixel_format) {
        if (not_inited_ptr(codec->pix_fmts)) {
            static_log_warning("utils", "compatible_with_pixel_format failed: codec->pix_fmts is NULL");
            return true;
        }

        auto pix_fmt = codec->pix_fmts;
        while (pix_fmt[0] != AV_PIX_FMT_NONE) {
            if (pix_fmt[0] == pixel_format) { return true; }
            pix_fmt++;
        }
        return false;
    }

    bool utils::compatible_with_sample_format(const AVCodec* codec, AVSampleFormat sample_format) {
        if (not_inited_ptr(codec->sample_fmts)) {
            static_log_warning("utils", "compatible_with_sample_format failed: codec->sample_fmts is NULL");
            return true;
        }

        auto smp_fmt = codec->sample_fmts;
        while (smp_fmt[0] != AV_SAMPLE_FMT_NONE) {
            if (smp_fmt[0] == sample_format) { return true; }
            smp_fmt++;
        }
        return false;
    }

    uid_t utils::gen_uid() {
        static std::atomic<uid_t> object_uid_handle = 0;
        return object_uid_handle++;
    }

    uid_t utils::gen_stream_uid(uid_t context_uid, uid_t stream_index) {
        return (context_uid + 1) * 100 + stream_index;
    }

    uid_t utils::get_context_uid(uid_t stream_uid) {
        return stream_uid / 100;
    }

    const char* utils::guess_format_short_name(const std::string_view media_resurs_locator) {
        if (media_resurs_locator.find("rtsp://") != std::string_view::npos) {
            return "rtsp";
        }
        if (media_resurs_locator.find("rtmp://") != std::string_view::npos) {
            return "flv";
        }
        if (media_resurs_locator.find("aevalsrc=") != std::string_view::npos) {
            return "lavfi";
        }
        if (media_resurs_locator.find("anullsrc=") != std::string_view::npos) { /* Silence              */
            return "lavfi";
        }
        if (media_resurs_locator.find("sine=") != std::string_view::npos) {     /* Hum/squeak           */
            return "lavfi";
        }
        if (media_resurs_locator.find("video=") != std::string_view::npos) {    /* USB camera's video   */
            return "dshow";
        }
        if (media_resurs_locator.find("audio=") != std::string_view::npos) {    /* USB micro's audio    */
            return "TODO 13.01";
        }
        return nullptr;
    }

    std::string utils::option_set_error_to_string(int ret) {
        if (AVERROR_OPTION_NOT_FOUND == ret) {
            return "av_opt_set failed: no matching option exists";
        }
        if (AVERROR(ERANGE) == ret) {
            return "av_opt_set failed: the value is out of range";
        }
        if (AVERROR(EINVAL) == ret) {
            return "av_opt_set failed: the value is not valid";
        }
        return "av_opt_set failed: unknown code: " + std::to_string(ret);
    }

    std::string utils::send_packet_error_to_string(int ret) {
        if (AVERROR(EAGAIN) == ret) {
            return "avcodec_send_packet failed: input is not accepted \
                    in the current state - user must read output with \
                    avcodec_receive_frame()";
        }
        if (AVERROR_EOF == ret) {
            return "avcodec_send_packet failed: the decoder has been \
                    flushed, and no new packets can be sent to it";
        }
        if (AVERROR(EINVAL) == ret) {
            return "avcodec_send_packet failed: codec not opened, \
                    it is an encoder, or requires flush";
        }
        if (AVERROR(ENOMEM) == ret) {
            return "avcodec_send_packet failed: failed to add packet \
                    to internal queue, or similar other errors: \
                    legitimate decoding errors";
        }
        return "avcodec_send_packet failed: unknown code: " + std::to_string(ret);
    }

    std::string utils::receive_frame_error_to_string(int ret) {
        if (AVERROR(EAGAIN) == ret) {
            return "avcodec_receive_frame failed: output is not available \
                    in this state - user must try to send new input";
        }
        if (AVERROR_EOF == ret) {
            return "avcodec_receive_frame failed: the decoder has been fully \
                    flushed, and there will be no more output frames";
        }
        if (AVERROR(EINVAL) == ret) {
            return "avcodec_receive_frame failed: codec not opened, or it \
                    is an encoder other negative values: \
                    legitimate decoding errors";
        }
        return "avcodec_receive_frame failed: unknown code: " + std::to_string(ret);
    }

    std::string utils::send_frame_error_to_string(int ret) {
        if (AVERROR(EAGAIN) == ret) {
            return "avcodec_receive_frame failed: input is not accepted in "
                    "the current state - user must read output "
                    "with avcodec_receive_packet()";
        }
        if (AVERROR_EOF == ret) {
            return "avcodec_receive_frame failed: the encoder has been flushed, "
                    "and no new frames can be sent to it";
        }
        if (AVERROR(EINVAL) == ret) {
            return "avcodec_receive_frame failed: codec not opened, "
                    "refcounted_frames not set, it is a decoder, or requires flush";
        }
        if (AVERROR(ENOMEM) == ret) {
            return "avcodec_receive_frame failed: failed to add packet "
                    "to internal queue, or similar other errors: "
                    "legitimate decoding errors";
        }
        return "avcodec_send_frame failed: unknown code: " + std::to_string(ret);
    }

    std::string utils::receive_packet_error_to_string(int ret) {
        if (AVERROR(EAGAIN) == ret) {
            return "avcodec_receive_frame failed: output is not available "
                    "in the current state - user must try to send input";
        }
        if (AVERROR_EOF == ret) {
            return "avcodec_receive_frame failed: the encoder has been "
                    "fully flushed, and there will be no more output packets";
        }
        if (AVERROR(EAGAIN) == ret) {
            return "avcodec_receive_frame failed: codec not opened, "
                    "or it is an encoder other errors: "
                    "lgitimate decoding errors";
        }
        return "avcodec_receive_packet failed: unknown code: " + std::to_string(ret);
    }

    SharedParameters utils::make_youtube_video_params() {
        const auto params { fpp::VideoParameters::make_shared() };
        params->setEncoder(AVCodecID::AV_CODEC_ID_H264);
        params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV420P);
        params->setTimeBase(DEFAULT_TIME_BASE);
        params->setGopSize(12);
        return params;
    }

    SharedParameters utils::make_youtube_audio_params() {
        const auto params { fpp::AudioParameters::make_shared() };
        params->setEncoder(AVCodecID::AV_CODEC_ID_AAC);
        params->setSampleFormat(AV_SAMPLE_FMT_FLTP);
        params->setTimeBase(DEFAULT_TIME_BASE);
        params->setSampleRate(44'100);
        params->setBitrate(128 * 1024);
        params->setChannelLayout(AV_CH_LAYOUT_STEREO);
        params->setChannels(2);
        return params;
    }

    SharedParameters utils::make_params(MediaType type) {
        switch (type) {
        case MediaType::Video:
            return VideoParameters::make_shared();
        case MediaType::Audio:
            return AudioParameters::make_shared();
        default:
            throw std::invalid_argument {
                "make_params failed: invalid media type"
            };
        }
    }

    SharedParameters utils::make_params(AVMediaType type) {
        switch (type) {
        case AVMediaType::AVMEDIA_TYPE_VIDEO:
            return VideoParameters::make_shared();
        case AVMediaType::AVMEDIA_TYPE_AUDIO:
            return AudioParameters::make_shared();
        default:
            throw std::invalid_argument {
                "make_params failed: invalid media type"
            };
        }
    }

    //TODO
    void utils::params_to_avcodecpar(const SharedParameters params, AVCodecParameters* codecpar) {
        codecpar->codec_id = params->codecId();
        codecpar->bit_rate = params->bitrate();

        switch (params->type()) {
        case MediaType::Video: {
            codecpar->codec_type = AVMediaType::AVMEDIA_TYPE_VIDEO;
            auto video_parameters = dynamic_cast<VideoParameters*>(params.get());
            codecpar->width                  = int(video_parameters->width());
            codecpar->height                 = int(video_parameters->height());
    //        codec->sample_aspect_ratio    = video_parameters->sampl; //TODO
            codecpar->format                = int(video_parameters->pixelFormat());
            break;
        }
        case MediaType::Audio: {
            codecpar->codec_type = AVMediaType::AVMEDIA_TYPE_AUDIO;
            auto audio_parameters = dynamic_cast<AudioParameters*>(params.get());
            codecpar->channel_layout   = audio_parameters->channelLayout();
            codecpar->channels         = int(audio_parameters->channels());
            codecpar->sample_rate      = int(audio_parameters->sampleRate());
            break;
        }
        case MediaType::Unknown:
            break;
        }
    }

    bool utils::rescaling_required(const IOParams& params) {
        both_params_is_video(params);

        const auto in  { std::static_pointer_cast<const VideoParameters>(params.in)  };
        const auto out { std::static_pointer_cast<const VideoParameters>(params.out) };

        if (in->width() != out->width()) {
            static_log_warning("utils", "Rescaling required: width mismatch "
                               << in->width() << " != " << out->width());
            return true;
        }
        if (in->height() != out->height()) {
            static_log_warning("utils", "Rescaling required: height mismatch "
                               << in->height() << " != " << out->height());
            return true;
        }
        if (in->pixelFormat() != out->pixelFormat()) {
            static_log_warning("utils", "Rescaling required: pixel format mismatch "
                               << in->pixelFormat() << " != " << out->pixelFormat());
            return true;
        }

        return false;
    }

    bool utils::resampling_required(const IOParams& params) {
        both_params_is_audio(params);

        const auto in  { std::static_pointer_cast<const AudioParameters>(params.in)  };
        const auto out { std::static_pointer_cast<const AudioParameters>(params.out) };

        if (in->sampleRate() != out->sampleRate()) {
            static_log_warning("utils", "Resampling required: sample rate mismatch "
                               << in->sampleRate() << " != " << out->sampleRate());
            return true;
        }
        if (in->sampleFormat() != out->sampleFormat()) {
            static_log_warning("utils", "Resampling required: sample format mismatch "
                               << in->sampleFormat() << " != " << out->sampleFormat());
            return true;
        }
        if (in->channels() != out->channels()) {
            static_log_warning("utils", "Resampling required: channels mismatch "
                               << in->channels() << " != " << out->channels());
            return true;
        }
        if (in->channelLayout() != out->channelLayout()) {
            static_log_warning("utils", "Resampling required: channel layout mismatch "
                               << in->channelLayout() << " != " << out->channelLayout());
            return true;
        }

        return false;
    }

    bool utils::video_filter_required(const IOParams& params) {
        both_params_is_video(params);

        const auto in  { std::static_pointer_cast<const VideoParameters>(params.in)  };
        const auto out { std::static_pointer_cast<const VideoParameters>(params.out) };

        if (in->frameRate() != out->frameRate()) {
            static_log_warning("utils", "Video filter required: framerate mismatch "
                                           << to_string(in->frameRate())
                                           << " != "
                                           << to_string(out->frameRate())
            );
            return true;
        }

        return false;
    }

    bool utils::audio_filter_required(const IOParams&) {
        throw std::runtime_error { "audio_filter_required() is not implemented" };
    }

    bool utils::transcoding_required(const IOParams& params) {
        params_has_same_type(params);

        const auto in  { params.in  };
        const auto out { params.out };

        if (in->codecId() != out->codecId()) {
            static_log_warning("utils", "Transcoding required: codec id mismatch "
                                           << in->codecId()
                                           << " != "
                                           << out->codecId()
            );
            return true;
        }

        return false;
    }

    bool utils::compare_float(float a, float b) {
        const auto epsilon { 0.0001f };
        return ::fabs(a - b) < epsilon;
    }

} // namespace fpp
