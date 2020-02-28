#pragma once
#include <fpp/core/async/AsyncObject.hpp>
#include <vector>
#include <atomic>

namespace fpp {

    template <class T>
    class AsyncVector : public AsyncObject<std::vector<T>> {

    public:

        AsyncVector() = default;
        AsyncVector(const AsyncVector& other) = default;
        AsyncVector(const AsyncVector&& other) = default;
        AsyncVector(const std::vector<T>& raw_other) = default;
        AsyncVector(const std::vector<T>&& raw_other) = default;
        virtual ~AsyncVector() override = default;

        AsyncVector& operator=(const AsyncVector& other);
        AsyncVector& operator=(const AsyncVector&& other);

    };

} // namespace fpp

