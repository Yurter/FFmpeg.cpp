#pragma once
#include <fpp/base/FormatContext.hpp>
#include <fpp/format/OutputContext.hpp>

namespace fpp {

class OutputFormatContext : public FormatContext {

public:

    explicit OutputFormatContext(const std::string_view mrl = {}, const std::string_view format = {});
    OutputFormatContext(OutputContext* output_ctx, const std::string_view format);
    ~OutputFormatContext() override;

    void                setOutputFormat(AVOutputFormat* out_fmt);
    AVOutputFormat*     outputFormat();

    void                createStream(SpParameters params);
    void                copyStream(const SharedStream other);

    bool                write(Packet packet);
    bool                interleavedWrite(Packet& packet);

    void                flush();

    std::string         sdp();

private:

    void                createContext() override;
    bool                openContext(const Options& options) override;
    std::string         formatName() const override;
    void                closeContext() override;

    void                guessOutputFromat();
    AVOutputFormat*     guessFormatByName(const std::string_view short_name) const;
    AVOutputFormat*     guessFormatByUrl(const std::string_view url) const;
    void                writeHeader(const Options& options);
    void                writeTrailer();
    void                initStreamsCodecpar();
    void                parseStreamsTimeBase();

private:

    AVOutputFormat*     _output_format;

};

} // namespace fpp
