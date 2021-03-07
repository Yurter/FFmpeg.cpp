#pragma once
#include <fpp/stream/Stream.hpp>
#include <fpp/base/Parameters.hpp>
#include <sstream>

constexpr auto ffmpeg_4_0 { AV_VERSION_INT(56,14,100) };
constexpr auto ffmpeg_4_1 { AV_VERSION_INT(56,35,100) };

static_assert (LIBAVCODEC_VERSION_INT >= ffmpeg_4_1, "Use libavcodec 4.1 version or newer");

namespace fpp {

    using bytes = std::vector<std::uint8_t>;

    constexpr auto NOPTS_VALUE      { AV_NOPTS_VALUE };
    constexpr auto ERROR_EOF        { AVERROR_EOF };
    constexpr auto DEFAULT_RATIONAL { AVRational { 0, 1 } };
    constexpr auto DEFAULT_INT      { 0 };
    constexpr auto not_inited_q     { [](auto x) { return ::av_cmp_q(x, DEFAULT_RATIONAL) == 0; } };
    constexpr auto not_inited_int   { [](auto x) { return x == DEFAULT_INT; } };

    class utils {

    public:

        static std::string  ffmpeg_version();

        static void         device_register_all();

        static std::string  to_string(AVMediaType type);
        static std::string  to_string(AVCodecID codec_id);
        static std::string  to_string(AVRational rational);
        static std::string  to_string(AVSampleFormat value);
        static std::string  to_string(AVPixelFormat pxl_fmt);
        static std::string  to_string(bool value);
        static std::string  to_string(MediaType type);
        static std::string  pts_to_string(std::int64_t pts);
        static std::string  time_to_string(std::int64_t time_stamp, AVRational time_base);
        static std::string  channel_layout_to_string(int nb_channels, std::uint64_t channel_layout);

        static MediaType    to_media_type(AVMediaType type);
        static AVMediaType  from_media_type(MediaType type);

        static std::string  quoted(const std::string_view str, char delim = '"');

        static void         sleep_for(std::int64_t milliseconds);
        static void         sleep_for_ms(std::int64_t milliseconds);
        static void         sleep_for_sec(std::int64_t seconds);
        static void         sleep_for_min(std::int64_t minutes);

        static bool         rescaling_required(const InOutParams& params);
        static bool         resampling_required(const InOutParams& params);
        static bool         transcoding_required(const InOutParams& params);

        static bool         compare_float(float a, float b);
        static void         handle_exceptions(const Object* owner);

        static const std::string_view guess_format_short_name(const std::string_view media_resurs_locator);

        /* FFmpeg's error codes explanation */
        static std::string  option_set_error_to_string(int ret);
        static std::string  send_packet_error_to_string(int ret);
        static std::string  receive_frame_error_to_string(int ret);
        static std::string  send_frame_error_to_string(int ret);
        static std::string  receive_packet_error_to_string(int ret);
        static std::string  swr_convert_frame_error_to_string(int ret);

        static SpParameters make_params(MediaType type);
        static SpParameters make_params(AVMediaType type);
        static SpParameters make_youtube_video_params();
        static SpParameters make_youtube_audio_params();

        static std::string  merge_sdp_files(const std::string& sdp_one, const std::string& sdp_two);

        static bytes serializeAVPacket(const AVPacket& packet);
        static AVPacket deserializeAVPacket(const bytes& data);
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

} // namespace fpp
