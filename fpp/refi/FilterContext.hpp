#pragma once
#include <fpp/base/Frame.hpp>
#include <fpp/stream/VideoParameters.hpp>

struct AVFilterGraph;
struct AVFilterContext;
struct AVFilterContext;

namespace fpp {

    class FilterContext : public Object {

    public:

        FilterContext(SharedParameters parameters, const std::string& filters_descr);
        virtual ~FilterContext() override = default;

        FrameList           filter(Frame frame);
        std::string         description() const;

        const SharedParameters params;

    public:

        static const char   Separator = ',';
        static std::string  set_pts(float coef) { return "setpts=" + std::to_string(coef) + "*PTS"; }
        static std::string  keep_every_frame(int n) { return "select='not(mod(n," + std::to_string(n) + "))'"; }

        protected:

        void                init();

        virtual void        initBufferSource(){}// = 0;
        virtual void        initBufferSink(){}// = 0;

    private:

        const std::string   _filters_descr;

    protected:

        std::shared_ptr<AVFilterGraph>      _filter_graph;
        std::shared_ptr<AVFilterContext>    _buffersrc_ctx;
        std::shared_ptr<AVFilterContext>    _buffersink_ctx;

    };

} // namespace fpp