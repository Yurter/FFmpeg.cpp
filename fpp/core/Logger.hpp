#pragma once
#include <sstream>
#include <iomanip>
#include <functional>
#include <mutex>

namespace fpp {

    enum class LogLevel : uint8_t {
        Quiet,
        Error,
        Warning,
        Info,
    };

    class MessageHandler {

    public:

        explicit MessageHandler(LogLevel log_level);
        MessageHandler(const std::string_view caller_name, LogLevel log_level);
        ~MessageHandler();

        template<typename T>
        inline MessageHandler& operator<<(T&& data) {
            _ss << data;
            return *this;
        }

    private:

        const std::string _caller_name;
        const LogLevel _log_level;
        std::stringstream _ss;

    };

    class ConsoleHandler {

    public:

        ConsoleHandler(std::mutex& mutex, LogLevel log_level);
        ~ConsoleHandler();

    private:

        void setConsoleColor(LogLevel log_level) const;
        void resetConsoleColor() const;

        void* _h_stdout;
        std::lock_guard<std::mutex> _lock;

    };

    class Logger {

    public:

        static Logger&      instance();

        void                setLogLevel(LogLevel log_level);
        void                setFFmpegLogLevel(LogLevel log_level) const;
        void                setPrintCallback(std::function<void(LogLevel,const std::string&)> foo);

        void print(LogLevel log_level, const std::string_view message) const;
        void print(const std::string_view caller_name, LogLevel log_level, const std::string_view message) const;

    private:

        Logger();
        ~Logger();

        Logger(Logger const&)            = delete;
        Logger(Logger const&&)           = delete;
        Logger& operator=(Logger const&) = delete;

    private:

        bool                ignoreMessage(LogLevel message_log_level) const;
        std::string         currentTimeFormated() const;
        std::string         threadIdFormated() const;
        static void         log_callback(void* ptr, int level, const char* fmt, va_list vl);
        static LogLevel     convert_log_level(int ffmpeg_level);
        std::string_view    logLevelToString(LogLevel value) const;

    private:

        LogLevel            _log_level;
        mutable std::mutex  _print_mutex;
        std::function<void(LogLevel,std::string)> _print_func;

    };

    void set_log_level(LogLevel log_level);
    void set_ffmpeg_log_level(LogLevel log_level);

    // TODO (18.05)
    // error: C2280: "fpp::MessageHandler::MessageHandler(const fpp::MessageHandler &)":
    // предпринята попытка ссылки на удаленную функцию
//    const auto test_log_info {
//        [ h_msg = MessageHandler { LogLevel::Info } ]() mutable -> MessageHandler& {
//            return h_msg;
//        }
//    };

    inline MessageHandler static_log_info() {
        return MessageHandler { LogLevel::Info };
    }
    inline MessageHandler static_log_warning() {
        return MessageHandler { LogLevel::Warning };
    }
    inline MessageHandler static_log_error() {
        return MessageHandler { LogLevel::Error };
    }

} // namespace fpp
