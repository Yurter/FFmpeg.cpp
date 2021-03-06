#include "Frame.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavutil/imgutils.h>
}

namespace fpp {

Frame::Frame(Type type)
    : Media(type)
    , _time_base { DEFAULT_RATIONAL }
    , _stream_index { -1 } {
}

Frame::Frame(const Frame& other)
    : Frame(other.type()) {
    ref(other);
}

Frame::Frame(const AVFrame& frame, Media::Type type, AVRational time_base, int stream_index)
    : Frame(type) {
    ref(frame, time_base, stream_index);
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

std::int64_t Frame::pts() const {
    return raw().pts;
}

void Frame::setPts(std::int64_t pts) {
    raw().pts = pts;
}

void Frame::setTimeBase(AVRational time_base) {
    _time_base = time_base;
}

void Frame::setStreamIndex(int stream_index) {
    _stream_index = stream_index;
}

AVRational Frame::timeBase() const {
    return _time_base;
}

int Frame::streamIndex() const {
    return _stream_index;
}

bool Frame::keyFrame() const {
    return raw().key_frame == 1;
}

int Frame::nbSamples() const {
    return raw().nb_samples;
}

int Frame::size() const {
    if (isVideo()) {
        return ::av_image_get_buffer_size(
                AVPixelFormat(raw().format)
                , raw().width
                , raw().height
                , 32 /* align */
            );
    }
    if (isAudio()) {
        const auto channels {
            ::av_get_channel_layout_nb_channels(raw().channel_layout)
        };
        const auto bufer_size {
            ::av_samples_get_buffer_size(
                nullptr                        /* linesize */
                , channels
                , raw().nb_samples
                , AVSampleFormat(raw().format)
                , 32                           /* align */
            )
        };
        return bufer_size;
    }
    return 0;
}

std::string Frame::toString() const {
    /* Video frame: 460800 bytes, pts 1016370, key_frame false, width 640, height 480, yuv420p */
    std::string str = utils::to_string(type()) + " frame: ";
    if (isVideo()) {
        str += std::to_string(size()) + " bytes, "
                + '[' + ::av_get_picture_type_char(raw().pict_type) +']' + ", "
                + "pts " + utils::pts_to_string(raw().pts) + ", "
                + "width " + std::to_string(raw().width) + ", "
                + "height " + std::to_string(raw().height) + ", "
                + utils::to_string(AVPixelFormat(raw().format)) + ", "
                + "tb " + utils::to_string(timeBase()) + ", "
                + "stream index " + std::to_string(_stream_index);
        return str;
    }
    /* Audio frame: 1024 bytes, pts 425984, nb_samples 1024, channel_layout 4, sample_rate 44100 */
    if (isAudio()) {
        str += std::to_string(size()) + " bytes, "
                + (keyFrame() ? "[I]" : "[_]") + ", "
                + "pts " + utils::pts_to_string(raw().pts) + ", "
                + "samples " + std::to_string(raw().nb_samples) + ", "
                + "channel_layout " + utils::channel_layout_to_string(raw().channels, raw().channel_layout) + ", "
                + "sample_rate " + std::to_string(raw().sample_rate) + ", "
                + "tb " + utils::to_string(timeBase()) + ", "
                + "stream index " + std::to_string(_stream_index);
        return str;
    }
    /* Unknown frame: -1 bytes */
    {
        str += std::to_string(-1) + " bytes";
        return str;
    }
}

void Frame::ref(const Frame& other) {
    ref(other.raw(), other.timeBase(), other.streamIndex());
}

void Frame::ref(const AVFrame& other, AVRational time_base, int stream_index) {
    ffmpeg_api_strict(av_frame_ref, ptr(), &other);
    setTimeBase(time_base);
    setStreamIndex(stream_index);
}

void Frame::unref() {
    ::av_frame_unref(ptr());
}

} // namespace fpp
