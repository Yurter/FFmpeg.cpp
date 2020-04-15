#pragma once
#include <fpp/base/FormatContext.hpp>

namespace fpp {

    enum class SeekPrecision : uint8_t {
        Forward,
        Backward,
        Precisely,
        Any,
    };

    class InputFormatContext : public FormatContext {

    public:

        InputFormatContext(const std::string_view mrl = {}, const std::string_view format_short_name = {});
        ~InputFormatContext() override;

        void                seek(int64_t stream_index, int64_t timestamp, SeekPrecision seek_precision = SeekPrecision::Forward);
        Packet              read();

        static std::string  silence(int64_t sample_rate) {
            return "anullsrc=r=" + std::to_string(sample_rate)
                    + ":cl=mono";
        }

        static std::string  sine(int64_t frequency, int64_t sample_rate) {
            return "sine=frequency=" + std::to_string(frequency)
                    + ":sample_rate=" + std::to_string(sample_rate);
        }

    private:

        bool                openContext(Options options) override;
        std::string         formatName() const override;
        void                closeContext() override;

        [[nodiscard]]
        StreamVector        parseFormatContext() override;

        void                guessInputFromat();
        AVInputFormat*      findInputFormat(const std::string_view short_name) const;

        AVInputFormat*      inputFormat();
        void                setInputFormat(AVInputFormat* in_fmt);

    private:

        AVInputFormat*      _input_format;

    };

} // namespace fpp
