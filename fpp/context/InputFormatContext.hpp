#pragma once
#include <fpp/base/FormatContext.hpp>

namespace fpp {

    /* Точность поиска фрейма методом seek() */
    enum class SeekPrecision : uint8_t {
        Forward,
        Backward,
        Precisely,
        Any,
    };

    class InputFormatContext : public FormatContext {

    public:

        InputFormatContext(const std::string_view mrl);
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

        void                createContext() override;
        bool                openContext(Options options) override;
        std::string         formatName() const override;
        void                closeContext() override;

        [[nodiscard]]
        StreamVector        parseFormatContext() override;

        void                guessInputFromat();

        AVInputFormat*      inputFormat();
        void                setInputFormat(AVInputFormat* in_fmt);

    private:

        AVInputFormat*      _input_format;

    };

} // namespace fpp
