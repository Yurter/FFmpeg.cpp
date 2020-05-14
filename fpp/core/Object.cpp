#include "Object.hpp"
#include <cassert>

namespace fpp {

    std::string Object::name() const {
        /* return: fpp::Object */
        const auto name_with_namespace {
            std::string { typeid(*this).name() }
        };
        /* return: Object */
        const auto name_without_namespace {
            name_with_namespace.substr(name_with_namespace.find_last_of(':') + 1)
        };
        return name_without_namespace;
    }

    std::string Object::toString() const {
        return "[" + name() + ":" + std::to_string(int64_t(this)) + "]";
    }

} // namespace fpp
