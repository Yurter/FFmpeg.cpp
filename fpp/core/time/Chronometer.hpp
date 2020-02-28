#pragma once
#include <chrono>

namespace fpp {

    class Chronometer {

    public:

        Chronometer() {
            reset_timepoint();
        }

        void reset_timepoint() {
            _start_point = std::chrono::system_clock::now();
        }

        int64_t elapsed_milliseconds() const {
            auto end_point = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>
                    (end_point - _start_point).count();
        }

    private:

        using StartPoint = std::chrono::time_point<std::chrono::system_clock>;

        StartPoint          _start_point;

    };

} // namespace fpp
