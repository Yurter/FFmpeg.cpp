#pragma once
#include <fpp/stream/Stream.hpp>
#include <fpp/base/Parameters.hpp>
#include <sstream>

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace fpp {

    class utils {

    public:

        static uid_t        gen_uid();
        static uid_t        gen_stream_uid(uid_t context_uid, uid_t stream_index);
        static uid_t        get_context_uid(uid_t stream_uid);

        static std::string  to_string(AVMediaType type);
        static std::string  to_string(AVCodecID codec_id);
        static std::string  to_string(AVRational rational);
        static std::string  to_string(AVSampleFormat value);
        static std::string  to_string(AVPixelFormat pxl_fmt);
        static std::string  to_string(Code code);
        static std::string  to_string(bool value);
        static std::string  to_string(MediaType type);
        static std::string  pts_to_string(int64_t pts);
        static std::string  time_to_string(int64_t time_stamp, AVRational time_base);
        static std::string  channels_layout_to_string(int nb_channels, uint64_t channel_layout);

        static void         sleep_for(int64_t milliseconds);
        static void         sleep_for_ms(int64_t milliseconds);
        static void         sleep_for_sec(int64_t seconds);
        static void         sleep_for_min(int64_t minutes);

        static bool         rescaling_required(const IOParams& params);
        static bool         resampling_required(const IOParams& params);
        static bool         transcoding_required(const IOParams& params);
        static bool         video_filter_required(const IOParams& params);
        static bool         audio_filter_required(const IOParams& params);

        static bool         compare_float(float a, float b);

        static bool         exit_code(Code code);
        static bool         error_code(Code code);

        static bool         compatible_with_pixel_format(const AVCodec* codec, AVPixelFormat pixel_format);
        static bool         compatible_with_sample_format(const AVCodec* codec, AVSampleFormat sample_format);

        static const char*  guess_format_short_name(const std::string_view media_resurs_locator);

        /* FFmpeg's error codes explanation */
        static std::string  option_set_error_to_string(int ret);
        static std::string  send_packet_error_to_string(int ret);
        static std::string  receive_frame_error_to_string(int ret);
        static std::string  send_frame_error_to_string(int ret);
        static std::string  receive_packet_error_to_string(int ret);
        static std::string  swr_convert_frame_error_to_string(int ret);

        static SharedParameters make_params(MediaType type);
        static SharedParameters make_params(AVMediaType type);
        static SharedParameters make_youtube_video_params();
        static SharedParameters make_youtube_audio_params();

    };

    inline bool operator==(const AVRational& lhs, const AVRational& rhs) {
        return ::av_cmp_q(lhs, rhs) == 0;
    }

    inline bool operator!=(const AVRational& lhs, const AVRational& rhs) {
        return ::av_cmp_q(lhs, rhs) != 0;
    }

    inline std::ostream& operator<<(std::ostream& os, const AVRational rational) {
        os << utils::to_string(rational);
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const AVPixelFormat pix_fmt) {
        os << utils::to_string(pix_fmt);
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const AVSampleFormat smpl_fmt) {
        os << utils::to_string(smpl_fmt);
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const AVCodecID codec_id) {
        os << utils::to_string(codec_id);
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const MediaType type) {
        os << utils::to_string(type);
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const Code code) {
        os << utils::to_string(code);
        return os;
    }

} // namespace fpp

#define if_not(x)       if(!(x))
#define while_not(x)    while(!(x))
