#pragma once
#include <fpp/core/Object.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <memory>

namespace fpp {

    template<typename T>
    class SharedFFmpegObject : public Object {

    public:

        SharedFFmpegObject(std::shared_ptr<T> shared_object = std::shared_ptr<T> {})
            : _shared_object(shared_object) {
        }
        virtual ~SharedFFmpegObject() = default;

        void reset(std::shared_ptr<T> shared_object) {
            if (!shared_object) {
                throw std::runtime_error { "SharedFFmpegObject::reset() failed: data is null" };
            }
            _shared_object = shared_object;
        }

        T* raw() {
            if (!_shared_object) {
                throw std::runtime_error { "SharedFFmpegObject::raw() failed: object is null" };
            }
            return _shared_object.get();
        }

        const T* raw() const {
            if (!_shared_object) {
                throw std::runtime_error { "SharedFFmpegObject::raw() failed: object is null" };
            }
            return _shared_object.get();
        }

    private:

        std::shared_ptr<T> _shared_object;

    };

} // namespace fpp
