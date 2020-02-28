#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/Parameters.hpp>
#include <fpp/base/Dictionary.hpp>

struct AVCodecContext;

namespace fpp {

    class CodecContext : public SharedFFmpegObject<AVCodecContext> {

    public:

        CodecContext(const SharedParameters parameters);
        virtual ~CodecContext() override;

        std::string         toString() const override final;

        bool                opened() const;
        bool                closed() const;

        const AVCodec*      codec() const;
        virtual void        onOpen() { } //TODO убрать 24.01

        const SharedParameters  params;

    protected:

        void                init(Dictionary&& dictionary);

    private:

        void                open(Dictionary&& dictionary);
        void                close();
        void                setOpened(bool value);

        void                initContextParams();

    private:

        bool                _opened;

    };

} // namespace fpp
