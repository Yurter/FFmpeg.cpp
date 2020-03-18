#pragma once
#include <string_view>
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <vector>
#include <string>

struct AVDictionary;

namespace fpp {

    using Options = std::vector<std::pair<std::string,std::string>>;

    class Dictionary : public Object {

    public:

        Dictionary(Options options = Options {});

        void setOption(const std::string_view key, const std::string_view value);
        void setOption(const std::string_view key,                int64_t value);

        [[nodiscard]]
        AVDictionary*       alloc()                     const;
        void                free(AVDictionary* dict)    const;

    private:

        static void setString(AVDictionary** dict, const std::string_view key, const std::string_view value);
        static void setInt   (AVDictionary** dict, const std::string_view key,                int64_t value);

    private:

        Options _options;

    };

} // namespace fpp
