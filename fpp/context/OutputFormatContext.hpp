#pragma once
#include <fpp/base/FormatContext.hpp>

namespace fpp {

    enum class WriteMode : uint8_t {
        Instant,
        Interleaved,
    };

    class OutputFormatContext : public FormatContext {

    public:

        OutputFormatContext(const std::string_view mrl);
        ~OutputFormatContext() override;

        void                createStream(SharedParameters params);
        void                copyStream(const SharedStream other);

        bool                write(Packet packet, WriteMode write_mode = WriteMode::Instant);
        void                flush();

        std::string         sdp();

    private:

        void                createContext() override;
        bool                openContext(Options options) override;
        std::string         formatName() const override;
        void                closeContext() override;

        [[nodiscard]]
        StreamVector        parseFormatContext() override;

        Code                guessOutputFromat();
        void                writeHeader();
        void                writeTrailer();
        void                initStreamsCodecpar();
        void                parseStreamsTimeBase();

    private:

        AVOutputFormat*     outputFormat();
        void                setOutputFormat(AVOutputFormat* out_fmt);

    private:

        AVOutputFormat*     _output_format;

    };

} // namespace fpp
