#include "Dictionary.hpp"
#include <fpp/core/FFmpegException.hpp>

extern "C" {
    #include <libavutil/dict.h>
}

namespace fpp {

    Dictionary::Dictionary() {
        AVDictionary* raw_ptr { nullptr };
        if (const auto ret {
                ::av_dict_set(&raw_ptr, "dict_creation", "null", 0)
            }; ret < 0) {
            throw FFmpegException {
                "AVDictionary creation failed"
                , ret
            };
        }
        reset(SemiSmartPointer {
            raw_ptr
            , [](auto* dict) { ::av_dict_free(&dict); }
        });
    }

    void Dictionary::setOption(const std::string_view value, const std::string& key) {
        if (const auto ret {
                ::av_dict_set(ptrPtr(), value.data(), key.c_str(), 0) //TODO BUG!! прядок аргументов value key
            }; ret < 0) {
            throw FFmpegException {
                std::string("Setting option - ")
                    + value.data() + " " + key
                    + " failed"
                , ret
            };
        }
    }

    void Dictionary::setOption(const std::string_view value, int64_t key) {
        if (const auto ret {
                ::av_dict_set_int(ptrPtr(), value.data(), key, 0)
            }; ret < 0) {
            throw FFmpegException {
                std::string("Setting option - ")
                        + value.data() + " " + std::to_string(key)
                        + " failed"
                , ret
            };
        }
    }

} // namespace fpp
