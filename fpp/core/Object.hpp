#pragma once
#include <fpp/core/Logger.hpp>

#define inited_int(x)           ((x) != DEFAULT_INT)

namespace fpp {

    class Object {

    public:

        Object();
        virtual ~Object() = default;

        /*constexpr*/ void      setName(const std::string& name);
        std::string         name()          const;
        bool                is(const std::string& name) const;

        virtual std::string toString()      const;

        friend std::ostream& operator<<(std::ostream& os, const Object& object) {
            os << object.toString();
            return os;
        }

        friend std::ostream& operator<<(std::ostream& os, const Object*& object) {
            os << object->toString();
            return os;
        }

    protected:

        template <typename... Args>
        auto log_info(Args&&... args) const {
            Logger::instance().print(name(), LogLevel::Info, std::forward<Args>(args)...);
        }

        template <typename... Args>
        auto log_warning(Args&&... args) const {
            Logger::instance().print(name(), LogLevel::Warning, std::forward<Args>(args)...);
        }

        template <typename... Args>
        auto log_error(Args&&... args) const {
            Logger::instance().print(name(), LogLevel::Error, std::forward<Args>(args)...);
        }

    private:

        /*const*/ std::string   _name;
        bool                _inited;

    };

} // namespace fpp
