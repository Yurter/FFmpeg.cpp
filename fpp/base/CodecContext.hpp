#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/Dictionary.hpp>
#include <fpp/stream/Stream.hpp>

struct AVCodecContext;

namespace fpp {

    class CodecContext : public SharedFFmpegObject<AVCodecContext>, public MediaData {

    public:

        CodecContext(const SharedParameters params);
        virtual ~CodecContext() override;

        std::string         toString() const override final;

        bool                opened() const;
        bool                closed() const;

        const AVCodec*      codec() const;

        const SharedParameters params;

    protected:

        void                init(Options options);

    private:

        void                open(Options options);
        void                close();
        void                setOpened(bool value);

    private:

        bool                _opened;

    };

} // namespace fpp
