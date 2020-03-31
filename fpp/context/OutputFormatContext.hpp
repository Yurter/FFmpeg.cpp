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
        virtual ~OutputFormatContext() override;

        SharedStream        createStream(SharedParameters params);
        SharedStream        copyStream(
                                const SharedStream other
                                , SharedParameters output_params = SharedParameters {}
                            );

        void                write(Packet packet, WriteMode write_mode = WriteMode::Instant);
        void                flush();

        std::string         sdp();

    private:

        virtual void        createContext()         override;
        virtual bool        openContext(Options options) override;
        virtual std::string formatName() const      override;
        virtual void        beforeCloseContext()    override;

        [[nodiscard]]
        virtual StreamVector parseFormatContext() override;

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
