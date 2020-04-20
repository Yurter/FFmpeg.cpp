#include "AudioParameters.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/Logger.hpp>

extern "C" {
    #include <libavformat/avformat.h>
}

#define DEFAULT_SAMPLE_FORMAT   AV_SAMPLE_FMT_NONE
#define DEFAULT_CHANEL_LAYOUT   0
#define not_inited_smp_fmt(x)   ((x) == DEFAULT_SAMPLE_FORMAT)
#define not_inited_ch_layout(x) ((x) == DEFAULT_CHANEL_LAYOUT)

namespace fpp {

    AudioParameters::AudioParameters()
        : Parameters(MediaType::Audio) {
        setName("AudioParameters");
    }

    void AudioParameters::setSampleRate(int sample_rate) {
        raw().sample_rate = sample_rate;
    }

    void AudioParameters::setSampleFormat(AVSampleFormat sample_format) {
        raw().format = int(sample_format);
    }

    void AudioParameters::setChannelLayout(uint64_t channel_layout) {
        raw().channel_layout = channel_layout;
    }

    void AudioParameters::setChannels(int channels) {
        raw().channels = channels;
    }

    void AudioParameters::setFrameSize(int frame_size) {
        raw().frame_size = frame_size;
    }

    int AudioParameters::sampleRate() const {
        return raw().sample_rate;
    }

    AVSampleFormat AudioParameters::sampleFormat() const {
        return AVSampleFormat(raw().format);
    }

    uint64_t AudioParameters::channelLayout() const {
        return raw().channel_layout;
    }

    int AudioParameters::channels() const {
        return raw().channels;
    }

    int AudioParameters::frameSize() const {
        return raw().frame_size;
    }

    std::string AudioParameters::toString() const {
        return Parameters::toString() + ", "
            + "sample_rate " + std::to_string(sampleRate()) + ", "
            + utils::to_string(sampleFormat()) + ", "
            + "channel_layout " + utils::channel_layout_to_string(channels(), channelLayout()) + ", "
            + "channels " + std::to_string(channels());
    }

    void AudioParameters::completeFrom(const SpParameters other) {
        Parameters::completeFrom(other);
        const auto other_audio {
            std::static_pointer_cast<AudioParameters>(other)
        };
        if (not_inited_int(sampleRate()))           { setSampleRate(other_audio->sampleRate());         }
        if (not_inited_smp_fmt(sampleFormat()))     { setSampleFormat(other_audio->sampleFormat());     }
        if (not_inited_ch_layout(channelLayout()))  { setChannelLayout(other_audio->channelLayout());   }
        if (not_inited_int(channels()))             { setChannels(other_audio->channels());             }
    }

    void AudioParameters::parseStream(const AVStream* avstream) {
        Parameters::parseStream(avstream);
        setSampleRate(avstream->codecpar->sample_rate);
        setSampleFormat(AVSampleFormat(avstream->codecpar->format));
        setChannels(avstream->codecpar->channels);
        const auto channel_layout_unspecified {
            avstream->codecpar->channel_layout == 0
        };
        if (channel_layout_unspecified) {
            avstream->codecpar->channel_layout
                = uint64_t(::av_get_default_channel_layout(int(channels())));
        }
        setChannelLayout(avstream->codecpar->channel_layout);
        setFrameSize(avstream->codecpar->frame_size);
    }

    bool AudioParameters::betterThen(const SpParameters& other) {
        const auto other_audio { std::static_pointer_cast<AudioParameters>(other) };
        const auto this_sound_quality { sampleRate() * channels() };
        const auto other_sound_quality { other_audio->sampleRate() * other_audio->channels() };
        return this_sound_quality > other_sound_quality;
    }

} // namespace fpp
