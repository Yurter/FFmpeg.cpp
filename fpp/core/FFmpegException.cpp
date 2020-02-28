#include "FFmpegException.hpp"

extern "C" {
    #include <libavutil/error.h>
}

namespace fpp {

    FFmpegException::FFmpegException(const std::string& error_message, int ret)
        : _error_message(error_message + ": " + std::to_string(ret))
        , _ret(ret) {
    }

    const char* FFmpegException::what() const noexcept {
        return _error_message.c_str();
    }

    int FFmpegException::ret() const noexcept {
        return _ret;
    }

} // namespace fpp
