#include "AudioBufferSource.hpp"
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavfilter/buffersrc.h>
}

namespace fpp {

    AudioBufferSource::AudioBufferSource(const SpAudioParameters par
                                         , AVFilterGraph* graph
                                         , const std::string_view unique_id
                                         )
        : FilterContext(graph, "abuffer", unique_id, createArgs(par), nullptr)
        , _params { par } {
        setName("AudioBufferSrc");
    }

    void AudioBufferSource::write(const Frame& frame) {
        ffmpeg_api_strict(av_buffersrc_write_frame, raw(), frame.ptr());
    }

    std::string AudioBufferSource::createArgs(const SpAudioParameters par) const {
        std::stringstream ss;
        ss << "time_base=" << par->timeBase().num
                           << '/'
                           << par->timeBase().den;
        ss << ":sample_rate="    << par->sampleRate();
        ss << ":sample_fmt="     << par->sampleFormat();
        ss << ":channel_layout=" << par->channelLayout();
        ss << ":channels="       << par->channels();
        return ss.str();
    }

} // namespace fpp
