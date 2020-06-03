#include "FFmpegException.hpp"

namespace fpp {

    FFmpegException::FFmpegException(const std::string& error_message)
        : _error_message { error_message } {
    }

    const char* FFmpegException::what() const noexcept {
        return _error_message.c_str();
    }

} // namespace fpp
