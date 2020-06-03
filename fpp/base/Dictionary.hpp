#pragma once
#include <fpp/core/wrap/FFmpegObject.hpp>
#include <string_view>
#include <vector>
#include <string>

struct AVDictionary;

namespace fpp {

    using Entry = std::pair<std::string,std::string>;
    using Options = std::vector<Entry>;

    class Dictionary : public Object {

    public:

        explicit Dictionary(const Options& options);
        ~Dictionary() override;

        void setOption(const std::string_view key, const std::string_view value);
        void setOption(const std::string_view key,                int64_t value);

        AVDictionary**  get();

    private:

        [[nodiscard]]
        AVDictionary*   alloc(const Options&) const;
        void            free();

        void setString(AVDictionary** dict, const std::string_view key, const std::string_view value) const;
        void setInt   (AVDictionary** dict, const std::string_view key,                int64_t value) const;

    private:

        AVDictionary*       _dictionary;

    };

} // namespace fpp
