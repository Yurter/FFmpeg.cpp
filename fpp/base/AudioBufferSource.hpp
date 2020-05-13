#pragma once
#include <fpp/base/FilterContext.hpp>
#include <fpp/stream/AudioParameters.hpp>
#include <fpp/base/Frame.hpp>

namespace fpp {

    class AudioBufferSource : public FilterContext {

    public:

        explicit AudioBufferSource(const SpAudioParameters par
                                    , AVFilterGraph* graph
                                    , const std::string_view unique_id);

        void                write(const Frame& frame);

    private:

        std::string         createArgs(const SpAudioParameters par) const;

    private:

        const SpAudioParameters _params;

    };

} // namespace fpp
