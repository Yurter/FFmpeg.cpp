#pragma once
#include <chrono>

namespace fpp {

class Chronometer {

public:

    Chronometer() {
        reset();
    }

    void reset() {
        _start_point = std::chrono::steady_clock::now();
    }

    std::chrono::milliseconds elapsed_milliseconds() const {
        const auto end_point { std::chrono::steady_clock::now() };
        return std::chrono::duration_cast<std::chrono::milliseconds>
                (end_point - _start_point);
    }

private:

    std::chrono::time_point<std::chrono::steady_clock> _start_point;

};

} // namespace fpp
