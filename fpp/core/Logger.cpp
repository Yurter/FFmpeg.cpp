#include "Logger.hpp"
#include <fpp/core/Utils.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <regex>
#include <ctime>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace fpp {

    Logger::Logger() :
        _log_level(LogLevel::Info)
        , _print_func { [&](LogLevel log_level, const std::string& message) {
            ConsoleHandler handler { _print_mutex, log_level };
            std::cout << message << '\n';
        } } {
//        av_log_set_callback(log_callback); //TODO later
//        setFFmpegLogLevel(LogLevel::Info);
//        print("Logger", LogLevel::Info, "Logger opened");
    }

    Logger::~Logger() {
//        print("Logger", LogLevel::Info, "Logger closed");
        ::av_log_set_callback(nullptr);
    }

    std::string Logger::currentTimeFormated() const {
        const auto now { std::chrono::system_clock::now() };
        const auto in_time_t { std::chrono::system_clock::to_time_t(now) };

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");

        const auto ms {
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count() % 1000
        };
        auto ms_str { std::to_string(ms) };
        constexpr auto ms_max_length { 3 };
        ss << "." << ms_str.insert(0, ms_max_length - ms_str.length(), '0');

        return ss.str();
    }

    std::string Logger::threadIdFormated() const {
#ifdef _WIN32
        constexpr auto MAX_LENGTH { 5 };
        std::string thread_id = (std::stringstream() << std::this_thread::get_id()).str();
        thread_id.insert(0, MAX_LENGTH - thread_id.length(), '0');
        return thread_id;
#else
        //TODO: DOESNT COMPILE ON GCC (LINUX) // use -pthread linking option
        return "0000";
#endif
    }

    void Logger::log_callback(void* ptr, int level, const char* fmt, va_list vl) {
        va_list vl2;
        char line[1024];
        static int print_prefix = 1;

        va_copy(vl2, vl);
        ::av_log_default_callback(ptr, level, fmt, vl);
        ::av_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);
        va_end(vl2);

        for (auto& symbol : line) {
            if (symbol == '\n') {
                symbol = ' ';
                break;
            }
        }

//        static_log("FFmpeg", convert_log_level(level), line);
    }

    LogLevel Logger::convert_log_level(int ffmpeg_level) { //TODO
//        switch (ffmpeg_level) {
//        case AV_LOG_QUIET:
//            return LogLevel::Quiet;
//        case AV_LOG_PANIC:
//        case AV_LOG_FATAL:
//        case AV_LOG_ERROR:
//            return LogLevel::Error;
//        case AV_LOG_WARNING:
//            return LogLevel::Warning;
//        case AV_LOG_INFO:
//            return LogLevel::Info;
//        default:
//            return LogLevel::Quiet;
//        }
        if (ffmpeg_level < AV_LOG_INFO) {
            return LogLevel::Error;
        } else {
            return LogLevel::Quiet;
        }
    }

    std::string_view Logger::logLevelToString(LogLevel value) const {
        switch (value) {
            case LogLevel::Info:
                return std::string_view { "info" };
            case LogLevel::Warning:
                return std::string_view { "warn" };
            case LogLevel::Error:
                return std::string_view { "err " };
            case LogLevel::Quiet:
                return std::string_view { "    " };
            }
        return std::string_view { "?   " };
    }

    Logger& Logger::instance() {
        static Logger _logger;
        return _logger;
    }

    void Logger::setLogLevel(LogLevel log_level) {
        _log_level = log_level;
    }

    void Logger::setFFmpegLogLevel(LogLevel log_level) const {
        switch (log_level) {
        case LogLevel::Info:
            ::av_log_set_level(AV_LOG_INFO);
            break;
        case LogLevel::Warning:
            ::av_log_set_level(AV_LOG_WARNING);
            break;
        case LogLevel::Error:
            ::av_log_set_level(AV_LOG_ERROR);
            break;
        case LogLevel::Quiet:
            ::av_log_set_level(AV_LOG_QUIET);
            break;
        }
    }

    void Logger::setPrintCallback(std::function<void (LogLevel, const std::string&)> foo) {
        _print_func = foo;
    }

    void Logger::print(const std::string_view caller_name, LogLevel log_level, const std::string_view message) const {
        if (ignoreMessage(log_level)) {
            return;
        }

        std::stringstream ss;
        ss << '[' << logLevelToString(log_level).data() << ']'
           << '[' << threadIdFormated()                 << ']'
           << '[' << currentTimeFormated()              << ']'
           << '[' << caller_name.data()                 << ']'
           << ' ' << message.data();

        _print_func(log_level, ss.str());
    }

    void Logger::print(LogLevel log_level, const std::string_view message) const { // TODO: duplicate code (18.10)
        if (ignoreMessage(log_level)) {
            return;
        }

        std::stringstream ss;
        ss << '[' << logLevelToString(log_level).data() << ']'
           << '[' << threadIdFormated()                 << ']'
           << '[' << currentTimeFormated()              << ']'
           << ' ' << message.data();

        _print_func(log_level, ss.str());
    }

    bool Logger::ignoreMessage(LogLevel message_log_level) const {
        return (message_log_level == LogLevel::Quiet) || (message_log_level > _log_level);
    }

    ConsoleHandler::ConsoleHandler(std::mutex& mutex, LogLevel log_level) :
        _h_stdout {
        #ifdef _WIN32
            ::GetStdHandle(STD_OUTPUT_HANDLE)
        #else
            nullptr
        #endif
        }
        , _lock { mutex } {
        setConsoleColor(log_level);
    }

    ConsoleHandler::~ConsoleHandler() {
        resetConsoleColor();
    }

    void ConsoleHandler::setConsoleColor(LogLevel log_level) const {
        #ifdef _WIN32

        constexpr auto white  { FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE      };
        constexpr auto yellow { FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY };
        constexpr auto red    { FOREGROUND_RED|FOREGROUND_INTENSITY                  };

        switch (log_level) {
            case LogLevel::Quiet:
                return;
            case LogLevel::Info:
                ::SetConsoleTextAttribute(_h_stdout, white);
                return;
            case LogLevel::Warning:
                ::SetConsoleTextAttribute(_h_stdout, yellow);
                return;
            case LogLevel::Error:
                ::SetConsoleTextAttribute(_h_stdout, red);
                return;
        }

        #elif __linux__
            // https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal // TODO: 23.04
        #endif
    }

    void ConsoleHandler::resetConsoleColor() const {
        #ifdef _WIN32
        constexpr auto white { FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE };
        ::SetConsoleTextAttribute(_h_stdout, white);
        #elif __linux__
            // https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal // TODO: 23.04
        #endif
    }

    MessageHandler::MessageHandler(LogLevel log_level)
        : _caller_name { /* default */ }
        , _log_level { log_level } {
    }

    MessageHandler::MessageHandler(const std::string_view caller_name, LogLevel log_level)
        : _caller_name { caller_name }
        , _log_level { log_level } {
    }

    MessageHandler::~MessageHandler() { // TODO: refactor (19.05)
        if (_caller_name.empty()) {
            switch (_log_level) {
                case LogLevel::Info:
                    Logger::instance().print(LogLevel::Info,    _ss.str());
                    return;
                case LogLevel::Warning:
                    Logger::instance().print(LogLevel::Warning, _ss.str());
                    return;
                case LogLevel::Error:
                    Logger::instance().print(LogLevel::Error,   _ss.str());
                    return;
                case LogLevel::Quiet:
                    return;
            }
        }
        else {
            switch (_log_level) {
                case LogLevel::Info:
                    Logger::instance().print(_caller_name, LogLevel::Info,    _ss.str());
                    return;
                case LogLevel::Warning:
                    Logger::instance().print(_caller_name, LogLevel::Warning, _ss.str());
                    return;
                case LogLevel::Error:
                    Logger::instance().print(_caller_name, LogLevel::Error,   _ss.str());
                    return;
                case LogLevel::Quiet:
                    return;
            }
        }
    }

    void set_log_level(LogLevel log_level) {
        Logger::instance().setLogLevel(log_level);
    }

    void set_ffmpeg_log_level(LogLevel log_level) {
        Logger::instance().setFFmpegLogLevel(log_level);
    }

} // namespace fpp
