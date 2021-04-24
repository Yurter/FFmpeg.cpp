#pragma once
#include <fpp/base/FormatContext.hpp>
#include <fpp/format/InputContext.hpp>

namespace fpp {

enum class SeekPrecision : std::uint8_t {
    Forward,
    Backward,
    Any,
};

class InputFormatContext : public FormatContext {

public:

    explicit InputFormatContext(const std::string_view mrl = {}, const std::string_view format = {});
    InputFormatContext(InputContext* input_ctx, const std::string_view format);
    ~InputFormatContext() override;

    AVInputFormat*      inputFormat();
    void                setInputFormat(AVInputFormat* in_fmt);

    bool                seek(int stream_index, std::int64_t timestamp, SeekPrecision seek_precision = SeekPrecision::Forward);
    Packet              read();

private:

    void                createContext() override;
    bool                openContext(const Options& options) override;
    std::string         formatName() const override;
    void                closeContext() override;

    void                retrieveStreams();

    void                guessInputFromat();
    AVInputFormat*      findInputFormat(const std::string_view short_name) const;

    Packet              readFromSource();

private:

    AVInputFormat*      _input_format;

};

} // namespace fpp
