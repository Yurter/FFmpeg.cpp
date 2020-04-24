#pragma once
#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <mutex>

namespace fpp {

    /* Категории сообщений, которые выводятся в консоль.
     * Каждый последующий уровень включает в себя предыдущий */
    enum LogLevel {
        /* Сообщения не выводится */
        Quiet,
        /* Сообщения об ошибках */
        Error,
        /* Сообщения о некорректно установленных параметрах,
         * которые могут привести к проблемам */
        Warning,
        /* Стандартная информация */
        Info,
    };

    class Logger {

    public:

        static Logger&      instance();

        void                setLogLevel(LogLevel log_level);
        void                setFFmpegLogLevel(LogLevel log_level) const;
        void                setPrintCallback(std::function<void(LogLevel,const std::string&)> foo);

        template <typename... Args>
        void print(const std::string_view caller_name, LogLevel log_level, Args&&... args) const {
            if (ignoreMessage(log_level)) {
                return;
            }
            const auto formated_message {
                formatMessage(caller_name, log_level, (std::forward<Args>(args), ...))
            };
            _print_func(log_level, formated_message);
        }

    private:

        Logger();
        ~Logger();

        Logger(Logger const&)            = delete;
        Logger(Logger const&&)           = delete;
        Logger& operator=(Logger const&) = delete;

    private:

        struct ConsoleHandler {

            ConsoleHandler(std::mutex& mutex, LogLevel log_level);
            ~ConsoleHandler();

        private:

            void setConsoleColor(LogLevel log_level) const;
            void resetConsoleColor() const;

            void* _h_stdout;
            std::lock_guard<std::mutex> _lock;
        };

    private:

        template <typename... Args>
        std::string formatMessage(const std::string_view caller_name, LogLevel log_level, Args&&... args) const {
            std::stringstream ss;

            ss << '[' << logLevelToString(log_level) << ']'
               << '[' << threadIdFormated() << ']'
               << '[' << currentTimeFormated() << ']'
               << '[' << std::setw(15) << std::left << std::setfill(' ') << caller_name << ']' << ' ';

            (ss << ... << std::forward<Args>(args));

            return ss.str();
        }

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


/* Макрос установки дирректории, в которой находятся файлы лога.
 * Должен вызыватся первым по отношению к остальным макросам    */
//#define set_log_dir(x) fpp::Logger::instance(x)

/* Макрос установки уровня лога - сообщения, имеющие урень выше установленного, игнорируются */
//#define set_log_level(x)        logger.setLogLevel(x)
//#define set_ffmpeg_log_level(x) logger.setFFmpegLogLevel(x)

    inline auto set_log_level(LogLevel log_level) {
        Logger::instance().setLogLevel(log_level);
    }

    template <typename... Args>
    inline auto static_log_info(const std::string_view caller_name, Args&&... args) {
        Logger::instance().print(caller_name, LogLevel::Info, (std::forward<Args>(args), ...));
    }

    template <typename... Args>
    inline auto static_log_warning(const std::string_view caller_name, Args&&... args) {
        Logger::instance().print(caller_name, LogLevel::Warning, (std::forward<Args>(args), ...));
    }

    template <typename... Args>
    inline auto static_log_error(const std::string_view caller_name, Args&&... args) {
        Logger::instance().print(caller_name, LogLevel::Error, (std::forward<Args>(args), ...));
    }

} // namespace fpp
