#include "FilterGraph.hpp"
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/stream/AudioParameters.hpp>
#include <cassert>

extern "C" {
    #include <libavfilter/avfilter.h>
    #include <libavutil/opt.h>
}

namespace fpp {

    FilterGraph::FilterGraph(const Options& options)
        : _filter_uid { 0 } {
        setName("FilterGraph");
        if (const auto raw_ptr {
            ::avfilter_graph_alloc()
        }; !raw_ptr) {
            log_error("avfilter_graph_alloc failed!");
        }
        else {
            reset({
                raw_ptr
                , [](auto* graph) { ::avfilter_graph_free(&graph); }
            });
            Dictionary dictionary { options };
            ffmpeg_api_strict(av_opt_set_dict, raw(), dictionary.get());
        }
    }

    void FilterGraph::init() {
        ffmpeg_api_strict(avfilter_graph_config, raw(), nullptr);
    }

    std::string FilterGraph::genUniqueId() {
        return std::to_string(_filter_uid++);
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
                    ss << ":sample_rate="    << apar->sampleRate();
                    ss << ":sample_fmt="     << apar->sampleFormat();
                    ss << ":channel_layout=" << apar->channelLayout();
                    ss << ":channels="       << apar->channels();
                    return ss.str();
                }
            }()
        };
        return FilterContext { raw(), filter_name, genUniqueId(), args, nullptr };
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
                    ss << "pix_fmts=" << vpar->pixelFormat();
                    return ss.str();
                } else {
                    return std::string {};
                }
            }()
        };
        return FilterContext { raw(), filter_name, genUniqueId(), args, nullptr };
    }

    std::size_t FilterGraph::emplaceFilterChainBack(FilterChain chain) {
        _filters.push_back(chain);
        return _filters.size() - 1;
    }

    FilterChain& FilterGraph::chain(std::size_t index) {
        return _filters[index];
    }

    std::pair<const std::string, const std::string>
    FilterGraph::extractNameArgs(const std::string_view filter_descr) const {
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

    std::vector<FilterContext> FilterGraph::createFilterContexts(const std::vector<std::string>& filters) {
        std::vector<FilterContext> result;
        for (const std::string& filter_descr : filters) {
            const auto& [name, args] { extractNameArgs(filter_descr) };
            const auto unique_id { genUniqueId() };
            result.emplace_back(raw(), name, unique_id, args, nullptr);
        }
        return result;
    }

} // namespace fpp
