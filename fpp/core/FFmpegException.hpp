#pragma once
#include <exception>
#include <string>

namespace fpp {

    class FFmpegException : public std::exception {

    public:

        explicit FFmpegException(const std::string& error_message);

        const char*         what()  const noexcept override;

    protected:

        const std::string   _error_message;

    };

} // namespace fpp

#define CODE_POS std::string { __FUNCTION__ } + "():" + std::to_string(__LINE__) + " "

//#define ffmpeg_api(foo,...)                                 \
//    foo(__VA_ARGS__)                                        \

#define ffmpeg_api_non_strict(foo,...)                      \
    do {                                                    \
        if (const auto ret { foo(__VA_ARGS__) }; ret < 0) { \
            return false;                                   \
        }                                                   \
    } while (false)

#define ffmpeg_api_strict(foo,...)                          \
    do {                                                    \
        if (const auto ret { foo(__VA_ARGS__) }; ret < 0) { \
            throw fpp::FFmpegException {                    \
                #foo " failed: " + CODE_POS                 \
            };                                              \
        }                                                   \
    } while (false)
