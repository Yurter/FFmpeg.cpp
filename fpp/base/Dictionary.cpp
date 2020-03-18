#include "Dictionary.hpp"
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>

extern "C" {
    #include <libavutil/dict.h>
}

namespace fpp {

    Dictionary::Dictionary(Options options)
        : _options { options } {
        setName("Dictionary");
    }

    void Dictionary::setOption(const std::string_view key, const std::string_view value) {
        _options.push_back({ key.data(), value.data() });
    }

    void Dictionary::setOption(const std::string_view key, int64_t value) {
        _options.push_back({ key.data(), std::to_string(value) });
    }

    AVDictionary* Dictionary::alloc() const {
        AVDictionary* dictionary { nullptr };
        for (const auto& [key, value] : _options) {
            setString(&dictionary, key, value);
        }
        return dictionary;
    }

    void Dictionary::free(AVDictionary* dict) const {
        AVDictionaryEntry* entry { nullptr };
        if (dict) {
            // iterate over all entries in dictionary
            while ((entry = ::av_dict_get(dict, "", entry, AV_DICT_IGNORE_SUFFIX))) {
                static_log_warning(
                    "Dictionary"
                    , "Unused option: " << entry->key << " " << entry->value
                );
            }
            ::av_dict_free(&dict);
        }
    }

    void Dictionary::setString(AVDictionary** dict, const std::string_view key, const std::string_view value) {
        if (const auto ret {
                ::av_dict_set(
                    dict
                    , key.data()
                    , value.data()
                    , 0 /* flags */
                )
            }; ret < 0) {
            throw FFmpegException {
                std::string("Setting option - ")
                    + value.data() + " " + key.data()
                    + " failed"
                , ret
            };
        }
    }

    void Dictionary::setInt(AVDictionary** dict, const std::string_view key, int64_t value) {
        if (const auto ret {
                ::av_dict_set_int(
                    dict
                    , key.data()
                    , value
                    , 0 /* flags */
                )
            }; ret < 0) {
            throw FFmpegException {
                std::string("Setting option - ")
                    + key.data() + " " + std::to_string(value)
                    + " failed"
                , ret
            };
        }
    }

} // namespace fpp
