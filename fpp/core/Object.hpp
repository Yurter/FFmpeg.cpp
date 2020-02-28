#pragma once
#include <string>
#include <iostream>
#include <sstream>

#define INVALID_INT             -1
#define DEFAULT_INT             0
#define DEFAULT_STRING          "none"
#define DEFAULT_RATIONAL        AVRational { 0, 1 }

#define inited_int(x)           ((x) != DEFAULT_INT)
#define inited_ptr(x)           ((x) != nullptr)
#define invalid_int(x)          ((x) == INVALID_INT)
#define not_inited_int(x)       ((x) == DEFAULT_INT)
#define not_inited_ptr(x)       ((x) == nullptr)
#define not_inited_string(x)    ((x) == DEFAULT_STRING)
#define not_inited_q(x)         (av_cmp_q(x, DEFAULT_RATIONAL) == 0)

namespace fpp {

    /* Коды результата выполнения некоторых функций */
    enum class [[nodiscard]] Code: uint8_t {
        OK,
        ERR,
        EXIT,
        AGAIN,
        EXCEPTION,
        NOT_INITED,
        END_OF_FILE,
        FFMPEG_ERROR,
        INVALID_INPUT,
        NOT_IMPLEMENTED,
        INVALID_CALL_ORDER,
    };

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

    private:

        /*const*/ std::string   _name;
        bool                _inited;

    };

} // namespace fpp
