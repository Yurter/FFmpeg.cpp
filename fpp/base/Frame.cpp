#include "Frame.hpp"
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libavutil/imgutils.h>
}

namespace fpp {

    Frame::Frame(MediaType type)
        : MediaData(type)
        , _time_base { DEFAULT_RATIONAL } {
        setName("Frame");
    }

    Frame::Frame(const Frame& other)
        : Frame(other.type()) {
        ref(other);
    }

    Frame::Frame(const AVFrame& frame, AVRational time_base, MediaType type)
        : Frame(type) {
        ref(frame, time_base);
    }

    Frame::~Frame() {
        unref();
    }

    Frame& Frame::operator=(const Frame& other) {
        unref();
        ref(other);
        setType(other.type());
        return *this;
    }

    int64_t Frame::pts() const {
        return raw().pts;
    }

    void Frame::setPts(int64_t pts) {
        raw().pts = pts;
    }

    void Frame::setTimeBase(AVRational time_base) {
        _time_base = time_base;
    }

    AVRational Frame::timeBase() const {
        return _time_base;
    }

    bool Frame::keyFrame() const {
        return raw().key_frame == 1;
    }

    size_t Frame::size() const {
        if (isVideo()) {
            return size_t(
                ::av_image_get_buffer_size(
                    AVPixelFormat(raw().format)
                    , raw().width
                    , raw().height
                    , 32 /* align */
                )
            );
        }
        if (isAudio()) {
            const auto channels {
                ::av_get_channel_layout_nb_channels(raw().channel_layout)
            };
            const auto bufer_size {
                ::av_samples_get_buffer_size(
                    nullptr                         /* linesize */
                    , channels
                    , raw().nb_samples
                    , AVSampleFormat(raw().format)
                    , 32                            /* align */
                )
            };
            return size_t(bufer_size);
        }
        return 0;
    }

    std::string Frame::toString() const {
        /* Video frame: 460800 bytes, pts 1016370, key_frame false, width 640, height 480, yuv420p */
        std::string str = utils::to_string(type()) + " frame: ";
        if (isVideo()) {
            str += std::to_string(size()) + " bytes, "
                    + (keyFrame() ? "[I]" : "[_]") + ", "
                    + "pts " + utils::pts_to_string(raw().pts) + ", "
                    + "width " + std::to_string(raw().width) + ", "
                    + "height " + std::to_string(raw().height) + ", "
                    + utils::to_string(AVPixelFormat(raw().format));
            return str;
        }
        /* Audio frame: 1024 bytes, pts 425984, nb_samples 1024, channel_layout 4, sample_rate 44100 */
        if (isAudio()) {
            str += std::to_string(size()) + " bytes, "
                    + (keyFrame() ? "[I]" : "[_]") + ", "
                    + "pts " + utils::pts_to_string(raw().pts) + ", "
                    + "samples " + std::to_string(raw().nb_samples) + ", "
                    + "channel_layout " + utils::channel_layout_to_string(raw().nb_samples, raw().channel_layout) + ", "
                    + "sample_rate " + std::to_string(raw().sample_rate);
            return str;
        }
        /* Unknown frame: -1 bytes */
        {
            str += std::to_string(-1) + " bytes";
            return str;
        }
    }

    void Frame::ref(const Frame& other) {
        if (::av_frame_ref(ptr(), other.ptr()) != 0) {
            throw FFmpegException { "av_packet_ref failed" };
        }
        setTimeBase(other.timeBase());
    }

    void Frame::ref(const AVFrame& other, AVRational time_base) {
        if (::av_frame_ref(ptr(), &other) != 0) {
            throw FFmpegException { "av_packet_ref failed" };
        }
        setTimeBase(time_base);
    }

    void Frame::unref() {
        ::av_frame_unref(ptr());
    }

} // namespace fpp
