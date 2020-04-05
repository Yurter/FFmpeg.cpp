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

#define CODE_POS std::string { __FUNCTION__ } + "():" + std::to_string(__LINE__) + " "

#define ffmpeg_api(foo,...) do {\
                                if (const auto ret { foo(__VA_ARGS__) }; ret < 0) {\
                                    throw fpp::FFmpegException {\
                                        #foo " failed: " + CODE_POS\
                                    };\
                                }\
                            } while (false)
