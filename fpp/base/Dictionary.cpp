#include "Dictionary.hpp"
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavutil/dict.h>
}

namespace fpp {

    Dictionary::Dictionary(const Options& options)
        : _dictionary { alloc(options) } {
        setName("Dictionary");
    }

    Dictionary::~Dictionary() {
        free();
    }

    void Dictionary::setOption(const std::string_view key, const std::string_view value) {
        setString(&_dictionary, key, value);
    }

    void Dictionary::setOption(const std::string_view key, int64_t value) {
        setString(&_dictionary, key, std::to_string(value));
    }

    AVDictionary** Dictionary::get() {
        return &_dictionary;
    }

    AVDictionary* Dictionary::alloc(const Options& options) const {
        AVDictionary* dictionary { nullptr };
        for (const auto& [key, value] : options) {
            setString(&dictionary, key, value);
        }
        return dictionary;
    }

    void Dictionary::free() {
        if (_dictionary) {
            // iterate over all entries in dictionary
            AVDictionaryEntry* entry { nullptr };
            while ((entry = ::av_dict_get(_dictionary, "", entry, AV_DICT_IGNORE_SUFFIX))) {
                static_log_warning(
                    "Dictionary"
                    , "Unused option: ", entry->key, " ", entry->value
                );
            }
            ::av_dict_free(&_dictionary);
        }
    }

    void Dictionary::setString(AVDictionary** dict, const std::string_view key, const std::string_view value) const {
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
            };
        }
    }

    void Dictionary::setInt(AVDictionary** dict, const std::string_view key, int64_t value) const {
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
            };
        }
    }

} // namespace fpp
