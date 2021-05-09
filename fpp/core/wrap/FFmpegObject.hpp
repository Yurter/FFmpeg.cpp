#pragma once
#include <fpp/core/Object.hpp>

namespace fpp {

template<typename T>
class FFmpegObject : public Object {

public:

    FFmpegObject(T data = T {})
        : _object(data) {
    }

    void                setRaw(T data) { _object = data; }

    T&                  raw()       { return  _object;  }
    const T&            raw() const { return  _object;  }
    T*                  ptr()       { return &_object;  }
    const T*            ptr() const { return &_object;  }

private:

    T                   _object;

};

} // namespace fpp
