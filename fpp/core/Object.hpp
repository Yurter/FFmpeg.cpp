#pragma once
#include <fpp/core/Logger.hpp>

#define INVALID_INT             -1

#define inited_int(x)           ((x) != DEFAULT_INT)
#define inited_ptr(x)           ((x) != nullptr)
#define invalid_int(x)          ((x) == INVALID_INT)

namespace fpp {

    using uid_t = int64_t;

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
            Logger::instance().print(name(), LogLevel::Info, (std::forward<Args>(args), ...));
        }

        template <typename... Args>
        auto log_warning(Args&&... args) const {
            Logger::instance().print(name(), LogLevel::Warning, (std::forward<Args>(args), ...));
        }

        template <typename... Args>
        auto log_error(Args&&... args) const {
            Logger::instance().print(name(), LogLevel::Error, (std::forward<Args>(args), ...));
        }

    private:

        /*const*/ std::string   _name;
        bool                _inited;

    };

} // namespace fpp
