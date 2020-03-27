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
        , _width { 0 }
        , _height { 0 }
        , _sample_aspect_ratio { DEFAULT_RATIONAL }
        , _frame_rate { DEFAULT_RATIONAL }
        , _pixel_format { DEFAULT_PIXEL_FORMAT }
        , _field_order { AVFieldOrder::AV_FIELD_UNKNOWN }
        , _color_range { AVColorRange::AVCOL_RANGE_UNSPECIFIED }
        , _color_primaries { AVColorPrimaries::AVCOL_PRI_UNSPECIFIED }
        , _color_space { AVColorSpace::AVCOL_SPC_UNSPECIFIED }
        , _chroma_location { AVChromaLocation::AVCHROMA_LOC_UNSPECIFIED }
        , _video_delay { 0 }
        , _color_trc { AVColorTransferCharacteristic::AVCOL_TRC_UNSPECIFIED } {
        setName("VideoParameters");
    }

    void VideoParameters::setWidth(int64_t width) {
        _width = width - (width % 2);
    }

    void VideoParameters::setHeight(int64_t height) {
        _height = height - (height % 2);
    }

    void VideoParameters::setSampleAspectRatio(AVRational sample_aspect_ratio) {
        _sample_aspect_ratio = sample_aspect_ratio;
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
        _pixel_format = pixel_format;
    }

    void VideoParameters::setGopSize(int64_t gop_size) {
        _gop_size = gop_size;
    }

    int64_t VideoParameters::width() const {
        return _width;
    }

    int64_t VideoParameters::height() const {
        return _height;
    }

    AVRational VideoParameters::frameRate() const {
        return _frame_rate;
    }

    AVPixelFormat VideoParameters::pixelFormat() const {
        return _pixel_format;
    }

    AVRational VideoParameters::sampleAspectRatio() const {
        return _sample_aspect_ratio;
    }

    int64_t VideoParameters::gopSize() const {
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

    void VideoParameters::initCodecContext(AVCodecContext* codec_context) const {
        Parameters::initCodecContext(codec_context);
        codec_context->pix_fmt                = pixelFormat();
        codec_context->width                  = int(width());
        codec_context->height                 = int(height());
        codec_context->field_order            = _field_order;
        codec_context->color_range            = _color_range;
        codec_context->color_primaries        = _color_primaries;
        codec_context->color_trc              = _color_trc;
        codec_context->colorspace             = _color_space;
        codec_context->chroma_sample_location = _chroma_location;
        codec_context->sample_aspect_ratio    = sampleAspectRatio();
        codec_context->has_b_frames           = int(_video_delay);
        codec_context->gop_size               = int(gopSize());
    }

    void VideoParameters::parseCodecContext(const AVCodecContext* codec_context) {
        Parameters::parseCodecContext(codec_context);
        setPixelFormat(codec_context->pix_fmt);
        setWidth(codec_context->width);
        setHeight(codec_context->height);
        _field_order         = codec_context->field_order;
        _color_range         = codec_context->color_range;
        _color_primaries     = codec_context->color_primaries;
        _color_trc           = codec_context->color_trc;
        _color_space         = codec_context->colorspace;
        _chroma_location     = codec_context->chroma_sample_location;
        _sample_aspect_ratio = codec_context->sample_aspect_ratio;
        _video_delay         = codec_context->has_b_frames;
    }

    void VideoParameters::initCodecpar(AVCodecParameters* codecpar) const {
        Parameters::initCodecpar(codecpar);
        codecpar->format              = pixelFormat();
        codecpar->width               = int(width());
        codecpar->height              = int(height());
        codecpar->field_order         = _field_order;
        codecpar->color_range         = _color_range;
        codecpar->color_primaries     = _color_primaries;
        codecpar->color_trc           = _color_trc;
        codecpar->sample_aspect_ratio = sampleAspectRatio();
    }

    bool VideoParameters::betterThen(const SharedParameters& other) {
        const auto other_video { std::static_pointer_cast<VideoParameters>(other) };
        const auto this_picture_size { width() * height() };
        const auto other_picture_size { other_video->width() * other_video->height() };
        return this_picture_size > other_picture_size;
    }

} // namespace fpp
