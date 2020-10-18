#pragma once
#include <fpp/base/FormatContext.hpp>

namespace fpp {

    class OutputFormatContext : public FormatContext {

    public:

        explicit OutputFormatContext(const std::string_view mrl = {});
        ~OutputFormatContext() override;

        AVOutputFormat*     outputFormat();
        void                setOutputFormat(AVOutputFormat* out_fmt);

        void                createStream(SpParameters params);
        void                copyStream(const SharedStream other);

        bool                write(Packet& packet);
        bool                interleavedWrite(Packet& packet);

        void                flush();

        std::string         sdp();

    private:

        void                createContext() override;
        bool                openContext(const Options& options) override;
        std::string         formatName() const override;
        void                closeContext() override;

        void                guessOutputFromat();
        void                writeHeader();
        void                writeTrailer();
        void                initStreamsCodecpar();
        void                parseStreamsTimeBase();

    private:

        AVOutputFormat*     _output_format;

    };

} // namespace fpp
