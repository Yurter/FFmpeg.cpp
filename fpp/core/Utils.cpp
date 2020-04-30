#include "Utils.hpp"
#include <thread>
#include <atomic>
#include <algorithm>
#include <cassert>
#include <fpp/core/Logger.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/stream/AudioParameters.hpp>

extern "C" {
    #include <libavutil/imgutils.h>
    #include <libavdevice/avdevice.h>
}

namespace fpp {

    std::string utils::to_string(MediaType type) {
        switch (type) {
        case MediaType::Unknown:
            return "Unknown";
        case MediaType::Video:
            return "Video";
        case MediaType::Audio:
            return "Audio";
        case MediaType::Subtitle:
            return "Subtitle";
        case MediaType::Data:
            return "Data";
        case MediaType::Attachment:
            return "Attachment";
        case MediaType::EndOF:
            return "EOF";
        }
        return "Invalid";
    }

    std::string utils::to_string(AVMediaType type) {
        switch (type) {
        case AVMediaType::AVMEDIA_TYPE_UNKNOWN:
            return "Unknown";
        case AVMediaType::AVMEDIA_TYPE_VIDEO:
            return "Video";
        case AVMediaType::AVMEDIA_TYPE_AUDIO:
            return "Audio";
        case AVMediaType::AVMEDIA_TYPE_DATA:
            return "Data";
        case AVMediaType::AVMEDIA_TYPE_SUBTITLE:
            return "Subtitle";
        case AVMediaType::AVMEDIA_TYPE_ATTACHMENT:
            return "Attachment";
        default:
            return "Invalid";
        }
    }

    std::string utils::pts_to_string(int64_t pts) {
        return (pts == NOPTS_VALUE) ? "NOPTS" : std::to_string(pts);
    }

    std::string utils::to_string(bool value) {
        return value ? "true" : "false";
    }

    std::string utils::to_string(AVPixelFormat pxl_fmt) {
        const auto ret { ::av_get_pix_fmt_name(pxl_fmt) };
        if (!ret) {
            return "NONE";
        }
        return std::string { ret };
    }

    std::string utils::to_string(AVSampleFormat value) {
        const auto ret { ::av_get_sample_fmt_name(value) };
        if (!ret) {
            return "NONE";
        }
        return std::string { ret };
    }

    std::string utils::to_string(AVCodecID codec_id) {
        return ::avcodec_get_name(codec_id);
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
        const auto time_ms { ::av_rescale_q(time_stamp, time_base, DEFAULT_TIME_BASE) };
        const auto ms {   time_ms % 1000               };
        const auto ss {  (time_ms / 1000) % 60         };
        const auto mm { ((time_ms / 1000) % 3600) / 60 };
        const auto hh {  (time_ms / 1000) / 3600       };
        return std::to_string(hh) + ':' + std::to_string(mm) + ':' + std::to_string(ss) + '.' + std::to_string(ms);
    }

    std::string utils::channel_layout_to_string(int nb_channels, uint64_t channel_layout) {
        if (channel_layout == 0) {
            return "Unknown or unspecified";
        }
        constexpr auto buf_size { 32 };
        char buf[buf_size];
        ::av_get_channel_layout_string(buf, buf_size, nb_channels, channel_layout);
        return std::string { buf };
    }

    MediaType utils::to_media_type(AVMediaType type) {
        switch (type) {
            case AVMediaType::AVMEDIA_TYPE_VIDEO:
                return MediaType::Video;
            case AVMediaType::AVMEDIA_TYPE_AUDIO:
                return MediaType::Audio;
            case AVMediaType::AVMEDIA_TYPE_SUBTITLE:
                return MediaType::Subtitle;
            case AVMediaType::AVMEDIA_TYPE_DATA:
                return MediaType::Data;
            case AVMediaType::AVMEDIA_TYPE_ATTACHMENT:
                return MediaType::Attachment;
            case AVMediaType::AVMEDIA_TYPE_UNKNOWN:
                return MediaType::Unknown;
            default: {
                throw std::invalid_argument {
                    std::string { __FUNCTION__ } + " failed, bad type "
                        + std::to_string(int(type))
                };
            }
        }
    }

    AVMediaType utils::from_media_type(MediaType type) {
        switch (type) {
            case MediaType::Video:
                return AVMediaType::AVMEDIA_TYPE_VIDEO;
            case MediaType::Audio:
                return AVMediaType::AVMEDIA_TYPE_AUDIO;
            case MediaType::Subtitle:
                return AVMediaType::AVMEDIA_TYPE_SUBTITLE;
            case MediaType::Data:
                return AVMediaType::AVMEDIA_TYPE_DATA;
            case MediaType::Attachment:
                return AVMediaType::AVMEDIA_TYPE_ATTACHMENT;
            default: {
                throw std::invalid_argument {
                    std::string { __FUNCTION__ } + " failed, bad type "
                        + std::to_string(int(type))
                };
            }
        }
    }

    std::string utils::quoted(const std::string_view str, char delim) {
        return delim + std::string { str } + delim;
    }

    std::string utils::to_string(AVRational rational) {
        return std::to_string(rational.num)
                + "/" +
                std::to_string(rational.den);
    }

    bool utils::compatible_with_pixel_format(const AVCodec* codec, AVPixelFormat pixel_format) {
        if (!codec) {
            throw std::runtime_error {
                std::string { __FUNCTION__ } +" failed: codec is NULL"
            };
        }
        if (!codec->pix_fmts) {
            static_log_warning(
                "utils"
                , std::string { __FUNCTION__ } +" failed: codec->pix_fmts is NULL"
            );
            return true;
//            throw std::runtime_error {
//               std::string { __FUNCTION__ } +" failed: codec->pix_fmts is NULL"
//            };
        }

        auto pix_fmt { codec->pix_fmts };
        while (pix_fmt[0] != AV_PIX_FMT_NONE) {
            if (pix_fmt[0] == pixel_format) {
                return true;
            }
            pix_fmt++;
        }
        return false;
    }

    bool utils::compatible_with_sample_format(const AVCodec* codec, AVSampleFormat sample_format) {
        if (!codec) {
            throw std::runtime_error {
                std::string { __FUNCTION__ } +" failed: codec is NULL"
            };
        }
        if (!codec->sample_fmts) {
            static_log_warning(
                "utils"
                , std::string { __FUNCTION__ } +" failed: codec->sample_fmts is NULL"
            );
            return true;
//            throw std::runtime_error {
//                std::string { __FUNCTION__ } +" failed: codec->sample_fmts is NULL"
//            };
        }

        auto smp_fmt { codec->sample_fmts };
        while (smp_fmt[0] != AV_SAMPLE_FMT_NONE) {
            if (smp_fmt[0] == sample_format) {
                return true;
            }
            smp_fmt++;
        }
        return false;
    }

    std::string utils::ffmpeg_version() {
        return std::string { ::av_version_info() };
    }

    void utils::device_register_all() {
        static struct CallOnce {
            CallOnce() { ::avdevice_register_all(); }
        }_;
    }

    const char* utils::guess_format_short_name(const std::string_view media_resurs_locator) {
        if (media_resurs_locator.find("rtsp://") != std::string_view::npos) {
            return "rtsp";
        }
        if (media_resurs_locator.find("rtp://") != std::string_view::npos) {
            return "rtp";
        }
        if (media_resurs_locator.find("rtmp://") != std::string_view::npos) {
            return "flv";
        }
        if (media_resurs_locator.find("aevalsrc=") != std::string_view::npos) {
            return "lavfi";
        }
        if (media_resurs_locator.find("anullsrc=") != std::string_view::npos) {
            return "lavfi";
        }
        if (media_resurs_locator.find("sine=") != std::string_view::npos) {
            return "lavfi";
        }
        if (media_resurs_locator.find("video=") != std::string_view::npos) {
            return "dshow";
        }
        if (media_resurs_locator.find("audio=") != std::string_view::npos) {
            return "dshow";
        }
        if (media_resurs_locator == "desktop") {
            return "gdigrab";
        }
        if (media_resurs_locator.find("concat") != std::string_view::npos) {
            return "concat";
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
        if (AVERROR_INVALIDDATA == ret) {
            return "avcodec_send_packet failed: Invalid data found "
                   "when processing input";
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

    std::string utils::swr_convert_frame_error_to_string(int ret) {
        if (ret & AVERROR_INPUT_CHANGED) {
            return "Input changed";
        }
        if (ret & AVERROR_OUTPUT_CHANGED) {
            return "Output changed";
        }
        return std::to_string(ret);
    }

    SpParameters utils::make_youtube_video_params() {
        const auto params { fpp::VideoParameters::make_shared() };
        params->setEncoder(AVCodecID::AV_CODEC_ID_H264);
        params->setPixelFormat(AVPixelFormat::AV_PIX_FMT_YUV420P);
        params->setTimeBase(DEFAULT_TIME_BASE);
        params->setGopSize(12);
        return params;
    }

    SpParameters utils::make_youtube_audio_params() {
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

    std::string utils::merge_sdp_files(const std::string& sdp_one, const std::string& sdp_two) {
        std::string result { sdp_one };

        std::istringstream iss(sdp_two);
        std::string line;

        while (std::getline(iss, line)) {
            if (result.find(line) == std::string::npos) {
                result.append(line + '\n');
            }
        }

        return result;
    }

    SpParameters utils::make_params(MediaType type) {
        switch (type) {
            case MediaType::Video:
                return VideoParameters::make_shared();
            case MediaType::Audio:
                return AudioParameters::make_shared();
            case MediaType::Subtitle:
                return Parameters::make_shared(MediaType::Subtitle);
            case MediaType::Data:
                return Parameters::make_shared(MediaType::Data);
            case MediaType::Attachment:
                return Parameters::make_shared(MediaType::Attachment);
            default:
                throw std::invalid_argument {
                    "make_params failed: invalid media type"
                };
        }
    }

    SpParameters utils::make_params(AVMediaType type) {
        switch (type) {
            case AVMediaType::AVMEDIA_TYPE_VIDEO:
                return VideoParameters::make_shared();
            case AVMediaType::AVMEDIA_TYPE_AUDIO:
                return AudioParameters::make_shared();
            case AVMediaType::AVMEDIA_TYPE_SUBTITLE:
                return Parameters::make_shared(MediaType::Subtitle);
            case AVMediaType::AVMEDIA_TYPE_DATA:
                return Parameters::make_shared(MediaType::Data);
            case AVMediaType::AVMEDIA_TYPE_ATTACHMENT:
                return Parameters::make_shared(MediaType::Attachment);
            default:
                throw std::invalid_argument {
                    "make_params failed: invalid media type"
                };
        }
    }

    bool utils::rescaling_required(const InOutParams& params) {
        assert(params.in->isVideo() && params.out->isVideo());

        const auto in  { std::static_pointer_cast<const VideoParameters>(params.in)  };
        const auto out { std::static_pointer_cast<const VideoParameters>(params.out) };

        if (in->width() != out->width()) {
            static_log_warning("utils", "Rescaling required: width mismatch "
                               , in->width(), " != " , out->width());
            return true;
        }
        if (in->height() != out->height()) {
            static_log_warning("utils", "Rescaling required: height mismatch "
                               , in->height(), " != " , out->height());
            return true;
        }
        if (in->pixelFormat() != out->pixelFormat()) {
            static_log_warning("utils", "Rescaling required: pixel format mismatch "
                               , in->pixelFormat(), " != " , out->pixelFormat());
            return true;
        }

        return false;
    }

    bool utils::resampling_required(const InOutParams& params) {
        assert(params.in->isAudio() && params.out->isAudio());

        const auto in  { std::static_pointer_cast<const AudioParameters>(params.in)  };
        const auto out { std::static_pointer_cast<const AudioParameters>(params.out) };

        if (in->sampleRate() != out->sampleRate()) {
            static_log_warning("utils", "Resampling required: sample rate mismatch "
                               , in->sampleRate(), " != ", out->sampleRate());
            return true;
        }
        if (in->sampleFormat() != out->sampleFormat()) {
            static_log_warning("utils", "Resampling required: sample format mismatch "
                               , in->sampleFormat(), " != ", out->sampleFormat());
            return true;
        }
        if (in->channels() != out->channels()) {
            static_log_warning("utils", "Resampling required: channels mismatch "
                               , in->channels(), " != ", out->channels());
            return true;
        }
        if (in->channelLayout() != out->channelLayout()) {
            static_log_warning("utils", "Resampling required: channel layout mismatch "
                               , in->channelLayout(), " != ", out->channelLayout());
            return true;
        }

        return false;
    }

    bool utils::video_filter_required(const InOutParams& params) {
        assert(params.in->isVideo() && params.out->isVideo());

        const auto in  { std::static_pointer_cast<const VideoParameters>(params.in)  };
        const auto out { std::static_pointer_cast<const VideoParameters>(params.out) };

        if (in->frameRate() != out->frameRate()) {
            static_log_warning("utils", "Video filter required: framerate mismatch "
                                           , to_string(in->frameRate())
                                           , " != "
                                           , to_string(out->frameRate())
            );
            return true;
        }

        return false;
    }

    bool utils::audio_filter_required(const InOutParams&) {
        throw std::runtime_error { "audio_filter_required() is not implemented" };
    }

    bool utils::transcoding_required(const InOutParams& params) {
        assert(params.in->type() == params.out->type());

        const auto in  { params.in  };
        const auto out { params.out };

        if (in->codecId() != out->codecId()) {
            static_log_warning("utils", "Transcoding required: codec id mismatch "
                                           , in->codecId()
                                           , " != "
                                           , out->codecId()
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
