#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/base/Dictionary.hpp>
#include <fpp/stream/Stream.hpp>

struct AVCodecContext;

namespace fpp {

    class CodecContext : public SharedFFmpegObject<AVCodecContext>, public MediaData {

    public:

        CodecContext(const SpParameters params);

        std::string         toString() const override final;

        const AVCodec*      codec()  const;
        bool                opened();

        const SpParameters params;

    protected:

        void                init(Options options);

    private:

        void                open(Options options);

    };

} // namespace fpp
