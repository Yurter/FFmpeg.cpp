#pragma once
#include <fpp/core/Object.hpp>
#include <memory>

namespace fpp {

template<typename T>
class SharedFFmpegObject : public Object {

public:

    template<typename D>
    void reset(T* raw_pointer, D dtor) {
        _shared_object = { raw_pointer, std::move(dtor) };
    }

    void reset() {
        _shared_object.reset();
    }

    auto raw() {
        if (!_shared_object) {
            throw std::runtime_error {
                std::string { __FUNCTION__ } + " failed: object is null"
            };
        }
        return _shared_object.get();
    }

    const T* raw() const {
        if (!_shared_object) {
            throw std::runtime_error {
                std::string { __FUNCTION__ } + " failed: object is null"
            };
        }
        return _shared_object.get();
    }

    auto unsafe_raw() {
        return _shared_object.get();
    }

    const T* unsafe_raw() const {
        return _shared_object.get();
    }

    bool isNull() const {
        return !_shared_object;
    }

private:

    std::shared_ptr<T> _shared_object;

};

} // namespace fpp
