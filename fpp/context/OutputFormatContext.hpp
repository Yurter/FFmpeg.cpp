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
        SharedStream        copyStream(const SharedStream other);

        void                write(Packet packet, WriteMode write_mode = WriteMode::Instant);
        void                flush();

    private:

        virtual void        createContext() override;
        virtual void        openContext()   override;
        virtual void        closeContext()  override;

        Code                guessOutputFromat();
        [[nodiscard]] virtual StreamVector parseFormatContext() override;

    private:

        AVOutputFormat*     outputFormat();
        void                setOutputFormat(AVOutputFormat* out_fmt);

    private:

        AVOutputFormat*     _output_format;

    };

} // namespace fpp
