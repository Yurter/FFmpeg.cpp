#pragma once
#include <mutex>

namespace fpp {

    template<class T>
    class AsyncObject {

    public:

        AsyncObject() = default;

        AsyncObject(const AsyncObject& other) {
            std::lock_guard lock(_mutex);
            other.access([this](const T& other_data){
                _data = other_data;
            });
        }

        AsyncObject(const AsyncObject&& other) {
            std::lock_guard lock(_mutex);
            other.access([this](const T& other_data){
                _data = std::move(other_data);
            });
        }

        AsyncObject& operator=(const AsyncObject& other) {
            std::lock_guard lock(_mutex);
            other.access([this](const T& other_data){
                _data = other_data;
            });
            return *this;
        }

        AsyncObject& operator=(const AsyncObject&& other) {
            std::lock_guard lock(_mutex);
            other.access([this](const T& other_data){
                _data = std::move(other_data);
            });
            return *this;
        }

        template<class F>
        void access(F&& foo) {
            std::lock_guard<std::mutex> lock(_mutex);
            foo(_data);
        }

    protected:

        T                   _data;
        mutable std::mutex  _mutex;

    };

} // namespace fpp
