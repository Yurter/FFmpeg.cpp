#pragma once
#include <fpp/core/Logger.hpp>

namespace fpp {

    class Object {

    public:

        Object();
        virtual ~Object() = default;

        /*constexpr*/ void      setName(const std::string& name);
        std::string         name()          const;
        bool                is(const std::string& name) const;

        virtual std::string toString()      const;

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

        /*const*/ std::string   _name; // TODO: change type to smth less heavy (40 bytes) (11.05)
//                                        maybe std::array<char,15> _name2; - 15 bytes
//                                        or const char* _name3; - 8 bytes
//                                        or std::string_view _name4; - 16 bytes

    };

} // namespace fpp
