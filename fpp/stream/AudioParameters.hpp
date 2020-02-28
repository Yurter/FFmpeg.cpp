#pragma once
#include <fpp/base/Parameters.hpp>

namespace fpp {

    class AudioParameters;
    using SharedAudioParameters = std::shared_ptr<AudioParameters>;

    class AudioParameters : public Parameters {

    public:

        AudioParameters();
        virtual ~AudioParameters() override = default;

        void                setSampleRate(int64_t sample_rate);
        void                setSampleFormat(AVSampleFormat sample_format);
        void                setChannelLayout(uint64_t channels_layout);
        void                setChannels(int64_t channels);
        void                setFrameSize(int64_t value);

        int64_t             sampleRate()    const;
        AVSampleFormat      sampleFormat()  const;
        uint64_t            channelLayout() const;
        int64_t             channels()      const;
        int64_t             frameSize()     const;

        std::string         toString() const override;

        virtual void        completeFrom(const SharedParameters other)  override;
        virtual void        parseStream(const AVStream* avstream)       override;
        virtual bool        betterThen(const SharedParameters& other)   override;

        static SharedAudioParameters make_shared() {
            return std::make_shared<AudioParameters>();
        }

    private:

        int64_t             _sample_rate;
        AVSampleFormat      _sample_format;
        uint64_t            _channel_layout;
        int64_t             _channels;
        int64_t             _frame_size;

    };

    using SharedAudioParameters = std::shared_ptr<AudioParameters>;

} // namespace fpp
