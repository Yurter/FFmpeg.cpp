#include "VideoParameters.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

#define DEFAULT_PIXEL_FORMAT    AV_PIX_FMT_NONE
#define not_inited_pix_fmt(x)   ((x) == DEFAULT_PIXEL_FORMAT)

namespace fpp {

    VideoParameters::VideoParameters()
        : Parameters(MediaType::Video)
        , _gop_size { 0 }
        , _frame_rate { DEFAULT_RATIONAL } {
        setName("VideoParameters");
    }

    void VideoParameters::setWidth(int width) {
        raw().width = width - (width % 2);
    }

    void VideoParameters::setHeight(int height) {
        raw().height = height - (height % 2);
    }

    void VideoParameters::setSampleAspectRatio(AVRational sample_aspect_ratio) {
        raw().sample_aspect_ratio = sample_aspect_ratio;
    }

    void VideoParameters::setFrameRate(AVRational frame_rate) {
        if ((frame_rate.num * frame_rate.den) == 0) {
            log_error("setFrameRate failed: " << frame_rate);
            AVRational default_framerate = { 16, 1 };
            log_error("seted default value: " << default_framerate);
            _frame_rate = default_framerate;
            return;
        }
        _frame_rate = frame_rate;
    }

    void VideoParameters::setPixelFormat(AVPixelFormat pixel_format) {
        if (!utils::compatible_with_pixel_format(codec(), pixel_format)) {
            throw std::invalid_argument {
                utils::to_string(pixel_format)
                + " doesn't compatible with "
                + codecName()
            };
        }
        raw().format = int(pixel_format);
    }

    void VideoParameters::setGopSize(int gop_size) {
        _gop_size = gop_size;
    }

    int VideoParameters::width() const {
        return raw().width;
    }

    int VideoParameters::height() const {
        return raw().height;
    }

    AVRational VideoParameters::frameRate() const {
        return _frame_rate;
    }

    AVPixelFormat VideoParameters::pixelFormat() const {
        return AVPixelFormat(raw().format);
    }

    AVRational VideoParameters::sampleAspectRatio() const {
        return raw().sample_aspect_ratio;
    }

    int VideoParameters::gopSize() const {
        return _gop_size;
    }

    std::string VideoParameters::toString() const {
        return Parameters::toString() + ", "
            + std::to_string(width()) + "x" + std::to_string(height()) + ", "
            + utils::to_string(frameRate()) + " fps, "
            + "gop " + (gopSize() ? std::to_string(gopSize()) : "N/A")+ ", "
            + utils::to_string(pixelFormat());
    }

    void VideoParameters::completeFrom(const SharedParameters other) {
        Parameters::completeFrom(other);
        const auto other_video_parames { std::static_pointer_cast<VideoParameters>(other) };
        if (not_inited_int(width()))            { setWidth(other_video_parames->width());               }
        if (not_inited_int(height()))           { setHeight(other_video_parames->height());             }
        if (not_inited_q(frameRate()))          { setFrameRate(other_video_parames->frameRate());       }
        if (not_inited_pix_fmt(pixelFormat()))  { setPixelFormat(other_video_parames->pixelFormat());   }
        if (not_inited_int(gopSize()))          { setGopSize(other_video_parames->gopSize());           }
        if (not_inited_q(sampleAspectRatio()))  { setSampleAspectRatio(other_video_parames->sampleAspectRatio()); }
    }

    void VideoParameters::parseStream(const AVStream* avstream) {
        Parameters::parseStream(avstream);
        setWidth(avstream->codecpar->width);
        setHeight(avstream->codecpar->height);
        setFrameRate(avstream->avg_frame_rate);
        setPixelFormat(AVPixelFormat(avstream->codecpar->format));
        setSampleAspectRatio(avstream->codecpar->sample_aspect_ratio);
    }

    bool VideoParameters::betterThen(const SharedParameters& other) {
        const auto other_video { std::static_pointer_cast<VideoParameters>(other) };
        const auto this_picture_size { width() * height() };
        const auto other_picture_size { other_video->width() * other_video->height() };
        return this_picture_size > other_picture_size;
    }

} // namespace fpp
