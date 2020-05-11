#include "FFmpegException.hpp"

namespace fpp {

    FFmpegException::FFmpegException(const std::string& error_message, int ret)
        : _error_message { error_message + ": " + std::to_string(ret) } {
    }

    const char* FFmpegException::what() const noexcept {
        return _error_message.c_str();
    }

} // namespace fpp
