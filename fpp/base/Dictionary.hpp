#pragma once
#include <string_view>
#include <fpp/core/wrap/SemiSmartPointer.hpp>

struct AVDictionary;

namespace fpp {

    class Dictionary : public SemiSmartPointer<AVDictionary> {

    public:

        Dictionary();

        void setOption(const std::string_view value, const std::string& key);
        void setOption(const std::string_view value,            int64_t key);

    };

} // namespace fpp
