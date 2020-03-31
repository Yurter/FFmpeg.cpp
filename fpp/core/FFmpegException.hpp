#pragma once
#include <exception>
#include <string>

namespace fpp {

    class FFmpegException : public std::exception {

    public:

        FFmpegException(const std::string& error_message, int ret = 0);
        virtual ~FFmpegException() override = default;

        virtual const char* what()  const noexcept override;
        int                 ret()   const noexcept;

    protected:

        const std::string   _error_message;

    };

} // namespace fpp
