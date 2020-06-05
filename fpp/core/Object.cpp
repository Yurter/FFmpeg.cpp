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
        return "[" + name() + ":" + std::to_string(std::int64_t(this)) + "]";
    }

    MessageHandler Object::log_info() const {
        return MessageHandler { name(), LogLevel::Info };
    }

    MessageHandler Object::log_warning() const {
        return MessageHandler { name(), LogLevel::Warning };
    }

    MessageHandler Object::log_error() const {
        return MessageHandler { name(), LogLevel::Error };
    }

} // namespace fpp
