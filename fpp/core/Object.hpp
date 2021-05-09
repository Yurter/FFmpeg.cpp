#pragma once
#include <fpp/core/Logger.hpp>

namespace fpp {

class Object {

public:

    Object() = default;
    virtual ~Object() = default;

    std::string         name()      const;

    virtual std::string toString()  const;

protected:

    MessageHandler      log_info()    const;
    MessageHandler      log_warning() const;
    MessageHandler      log_error()   const;

};

} // namespace fpp
