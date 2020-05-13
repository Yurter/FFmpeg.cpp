#pragma once
#include <fpp/base/FilterContext.hpp>
#include <fpp/stream/AudioParameters.hpp>
#include <fpp/base/Frame.hpp>

namespace fpp {

    class AudioBufferSink : public FilterContext {

    public:

        explicit AudioBufferSink(const SpAudioParameters par
                                , AVFilterGraph* graph
                                , const std::string_view unique_id);

        FrameVector         read();

    private:

        Frame               createFrame()   const;

    private:

        const SpAudioParameters _params;

    };

} // namespace fpp
