#pragma once
#include <fpp/base/Parameters.hpp>

namespace fpp {

    class AudioParameters;
    using SharedAudioParameters = std::shared_ptr<AudioParameters>;

    class AudioParameters : public Parameters {

    public:

        AudioParameters();

        void                setSampleRate(int sample_rate);
        void                setSampleFormat(AVSampleFormat sample_format);
        void                setChannelLayout(uint64_t channels_layout);
        void                setChannels(int channels);
        void                setFrameSize(int frame_size);

        int                 sampleRate()    const;
        AVSampleFormat      sampleFormat()  const;
        uint64_t            channelLayout() const;
        int                 channels()      const;
        int                 frameSize()     const;

        std::string         toString() const override;

        void                completeFrom(const SharedParameters other) override;
        void                parseStream(const AVStream* avstream)      override;
        bool                betterThen(const SharedParameters& other)  override;

        static SharedAudioParameters make_shared() {
            return std::make_shared<AudioParameters>();
        }

    };

    using SharedAudioParameters = std::shared_ptr<AudioParameters>;

} // namespace fpp
