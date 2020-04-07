#pragma once
#include <chrono>

namespace fpp {

    class Chronometer {

    public:

        Chronometer() {
            reset();
        }

        void reset() {
            _start_point = std::chrono::system_clock::now();
        }

        int64_t elapsed_milliseconds() const {
            const auto end_point { std::chrono::system_clock::now() };
            return std::chrono::duration_cast<std::chrono::milliseconds>
                    (end_point - _start_point).count();
        }

    private:

        using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

        TimePoint           _start_point;

    };

} // namespace fpp
