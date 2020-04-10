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

        Dictionary(const Options& options);
        virtual ~Dictionary() override;

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
