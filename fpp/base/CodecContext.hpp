#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/Dictionary.hpp>
#include <fpp/stream/Stream.hpp>

struct AVCodecContext;

namespace fpp {

    class CodecContext : public SharedFFmpegObject<AVCodecContext> { // TODO унаследовать от MediaType ? 16.03

    public:

        CodecContext(const SharedStream stream);
        virtual ~CodecContext() override;

        std::string         toString() const override final;

        bool                opened() const;
        bool                closed() const;

        const AVCodec*      codec() const;

        const SharedParameters params;

    protected:

        void                init(Dictionary dictionary);

    private:

        void                open(Dictionary dictionary);
        void                close();
        void                setOpened(bool value);

        void                initContext();
        void                initStreamCodecpar();

    private:

        const SharedStream  _stream;
        bool                _opened;

    };

} // namespace fpp
