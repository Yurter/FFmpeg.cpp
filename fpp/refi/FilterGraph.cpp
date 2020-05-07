#include "FilterGraph.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/stream/AudioParameters.hpp>
#include <cassert>
#include <memory>

extern "C" {
    #include <libavfilter/avfilter.h>
    #include <libavfilter/buffersrc.h>
    #include <libavfilter/buffersink.h>
    #include <libavutil/opt.h>
}

namespace fpp {

    fpp::FilterGraph::FilterGraph()
        : _filter_uid { 0 } {
        setName("FilterGraph");
        if (const auto raw_ptr {
            ::avfilter_graph_alloc()
        }; !raw_ptr) {
            log_error("avfilter_graph_alloc failed!");
        }
        else {
            { /* Костыль на количество потоков */ // TODO словарик 12.02
                raw_ptr->nb_threads = 1;
            }
            reset({
                raw_ptr
                , [](auto* graph) { ::avfilter_graph_free(&graph); }
            });
        }
    }

    std::size_t FilterGraph::createInputFilterChain(const SpParameters par, std::vector<std::string> filters) {
        FilterChain filter_chain { par->type(), par->timeBase() };

        filter_chain.filters.emplace_back(createBufferSource(par));
        for (const auto& filter_descr : filters) {
            const auto& [name, args] { splitFilterDescription(filter_descr) };
            const auto& unique_name  { genUniqueName() };
            filter_chain.filters.emplace_back(par->type(), par->timeBase(), raw(), name, unique_name, args, nullptr);

//            const AVSampleFormat smp_fmts[] {
//                AV_SAMPLE_FMT_FLTP
//                , AV_SAMPLE_FMT_NONE
//            };

//            const auto array_size {
//                int(
//                    ::av_int_list_length_for_size(
//                        sizeof(*(smp_fmts))
//                        , smp_fmts
//                        , uint64_t(AV_SAMPLE_FMT_NONE)
//                    )
//                    * sizeof(*(smp_fmts))
//                )
//            };

//            ffmpeg_api_strict(av_opt_set_bin
//                , filter_chain.filters.back().raw()
//                , "sample_fmts"
//                , reinterpret_cast<const uint8_t*>(smp_fmts)
//                , array_size
//                , AV_OPT_SEARCH_CHILDREN
//            );

//            av_opt_set_int_list(
//                filter_chain.filters.back().raw()
//                , "sample_fmts"
//                , ((int[]){ AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE })
//                , AV_SAMPLE_FMT_NONE
//                , AV_OPT_SEARCH_CHILDREN
//            );
        }
        linkChain(filter_chain);
        return emplaceFilterChainBack(std::move(filter_chain));
    }

    size_t FilterGraph::createOutputFilterChain(const SpParameters par, std::vector<std::string> filters) {
        FilterChain filter_chain { par->type(), par->timeBase() };
        for (const auto& filter_descr : filters) {
            const auto& [name, args] { splitFilterDescription(filter_descr) };
            const auto& unique_name  { genUniqueName() };
            filter_chain.filters.emplace_back(par->type(), par->timeBase(), raw(), name, unique_name, args, nullptr);
        }
        filter_chain.filters.emplace_back(createBufferSink(par));
        linkChain(filter_chain);
        return emplaceFilterChainBack(std::move(filter_chain));
    }

    void FilterGraph::createSimpleFilterChain(const SpParameters par, std::vector<std::string> filters) {
        FilterChain filter_chain { par->type(), par->timeBase() };
        filter_chain.filters.emplace_back(createBufferSource(par));
        for (const auto& filter_descr : filters) {
            const auto& [name, args] { splitFilterDescription(filter_descr) };
            const auto& unique_name  { genUniqueName() };
            filter_chain.filters.emplace_back(par->type(), par->timeBase(), raw(), name, unique_name, args, nullptr);
        }
        filter_chain.filters.emplace_back(createBufferSink(par));
        linkChain(filter_chain);
        emplaceFilterChainBack(std::move(filter_chain));
    }

    void FilterGraph::link(std::vector<std::size_t> in, std::vector<std::size_t> out) {
        for (const auto& idx_in : in) {
            for (const auto& idx_out : out) {
                _filters.at(idx_in).filters.back().linkTo(_filters.at(idx_out).filters.front());
            }
        }
    }

    void FilterGraph::write(const Frame& frame, std::size_t input_chain_index) {
        static bool inited = false;
        if (!inited) {
            ffmpeg_api_strict(avfilter_graph_config, raw(), nullptr);
            char* dump = avfilter_graph_dump(raw(), nullptr);
                av_log(NULL, AV_LOG_ERROR, "Graph :\n%s\n", dump);
            inited = true;
        }
        log_warning(frame.raw().format);
        _filters[input_chain_index].filters.front().write(frame);
    }

    FrameVector FilterGraph::read(size_t output_chain_index) {
        return _filters[output_chain_index].filters.back().read();
    }

    FilterContext FilterGraph::createBufferSource(const SpParameters par) {
        assert(par->isVideo() || par->isAudio());
        const auto filter_name {
            [&]() {
                return par->isVideo() ? "buffer" : "abuffer";
            }()
        };
        const auto args {
            [&]() {
                if (par->isVideo()) {
                    const auto vpar {
                        std::static_pointer_cast<const VideoParameters>(par)
                    };
                    std::stringstream ss;
                    ss << "video_size=" << vpar->width()
                                        << 'x'
                                        << vpar->height();
                    ss << ":pix_fmt=" << vpar->pixelFormat();
                    ss << ":time_base=" << vpar->timeBase().num
                                        << '/'
                                        << vpar->timeBase().den;
                    ss << ":pixel_aspect=" << vpar->sampleAspectRatio().num
                                           << '/'
                                           << vpar->sampleAspectRatio().den;
                    return ss.str();
                } else {
                    const auto apar {
                        std::static_pointer_cast<const AudioParameters>(par)
                    };
                    std::stringstream ss;
                    ss << "time_base=" << apar->timeBase().num
                                       << '/'
                                       << apar->timeBase().den;
                    ss << ":sample_rate=" << apar->sampleRate();
                    ss << ":sample_fmt=" << apar->sampleFormat();
                    ss << ":channel_layout=" << apar->channelLayout();
                    return ss.str();
                }
            }()
        };
        log_warning("args: ", args);
        return FilterContext { par->type(), par->timeBase(), raw(), filter_name, genUniqueName(), args, nullptr };
    }

    FilterContext FilterGraph::createBufferSink(const SpParameters par) {
        assert(par->isVideo() || par->isAudio());
        const auto filter_name {
            [&]() {
                return par->isVideo() ? "buffersink" : "abuffersink";
            }()
        };
        const auto args {
            [&]() {
                if (par->isVideo()) {
                    const auto vpar {
                        std::static_pointer_cast<const VideoParameters>(par)
                    };
                    std::stringstream ss;
                    ss << "pix_fmt=" << vpar->pixelFormat();
                    return ss.str();
                } else {
                    const auto apar {
                        std::static_pointer_cast<const AudioParameters>(par)
                    };
                    std::stringstream ss;
//                    ss << "sample_fmts=" << apar->sampleFormat();
                    return ss.str();
                }
            }()
        };
        return FilterContext { par->type(), par->timeBase(), raw(), filter_name, genUniqueName(), args, nullptr };
    }

    std::pair<const std::string, const std::string> FilterGraph::splitFilterDescription(const std::string_view filter_descr) const {
        if (const auto eq_pos {
            filter_descr.find('=')
        }; eq_pos != std::string::npos) {
            return {
                std::string { filter_descr.substr(0, eq_pos) }
                , std::string { filter_descr.substr(eq_pos + 1) }
            };
        }
        return { std::string { filter_descr }, std::string {} };
    }

    void FilterGraph::linkChain(FilterChain& chain) {
        for (std::size_t i { 0 }; i < chain.filters.size() - 1; ++i) {
            chain.filters.at(i).linkTo(chain.filters.at(i+1));
        }
    }

    std::string FilterGraph::genUniqueName() {
        return std::to_string(_filter_uid++);
    }

    std::size_t FilterGraph::emplaceFilterChainBack(FilterChain&& chain) {
        _filters.push_back(std::move(chain));
        return _filters.size() - 1;
    }

} // namespace fpp
