#pragma once
#include <functional>
#include <atomic>

namespace fpp {

    template <typename T, typename D = std::function<void(T*)>>
    class SemiSmartPointer {

    public:

        SemiSmartPointer(T* data = nullptr, D deleter = [](T* data) { delete data; })
            : _data { data }
            , _deleter { deleter } {
        }

        SemiSmartPointer(const SemiSmartPointer&& other)
            : _data { other._data }
            , _deleter { other._deleter } {
            other._data = nullptr;
        }

        SemiSmartPointer& operator=(SemiSmartPointer&& other) {
            _data = other._data;
            _deleter = other._deleter;
            other._data = nullptr;
            return *this;
        }

        ~SemiSmartPointer() {
            if (_data) {
                _deleter(_data);
            }
        }

        SemiSmartPointer(const SemiSmartPointer& other) = delete;
        SemiSmartPointer& operator=(const SemiSmartPointer& other) = delete;

        void reset(SemiSmartPointer&& other = SemiSmartPointer {}) {
            *this = std::move(other);
        }

        T*          raw()       { return _data;  }
        const T*    raw() const { return _data;  }
        T**         ptrPtr()    { return &_data; }


    private:

        T*                  _data;
        D                   _deleter;

    };

} // namespace fpp
