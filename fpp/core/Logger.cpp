#include "Logger.hpp"
#include <fpp/core/Utils.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <ctime>
#include <filesystem>
#include <regex>
#include <chrono>

namespace fpp {

    Logger::Logger()
        : _log_level(LogLevel::Info) {
        setName("Logger");
//        av_log_set_callback(log_callback); //TODO later
//        setFFmpegLogLevel(LogLevel::Info);
        print(this->name(), "CODE_POS", LogLevel::Info, "Logger opened");
    }

    Logger::~Logger() {
        print(this->name(), "CODE_POS", LogLevel::Info, "Logger closed");
        ::av_log_set_callback(nullptr);
    }

    void Logger::print(const LogLevel log_level, const std::string& log_text) const {
        ConsoleHandler handler { _print_mutex, log_level };
        std::cout << log_text << '\n';
    }

    std::string Logger::genFileName() const {
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];

        ::time(&rawtime);
        timeinfo = ::localtime(&rawtime);

        ::strftime(buffer, 80, "%Y-%m-%d_%H-%M-%S.txt", timeinfo);
        return std::string(buffer);
    }

    std::string Logger::formatMessage(std::string caller_name, const std::string& code_position, LogLevel log_level, const std::string& message) const {
        constexpr size_t max_name_length { 15 };
        caller_name.resize(max_name_length, ' ');

        const auto debug_log { log_level >= LogLevel::Debug };
        const auto trace_log { log_level >= LogLevel::Trace };

        const auto header {
            "[" + encodeLogLevel(log_level) + "]" +
            "[" + getTimeStamp() + "]" +
            "[" + caller_name + "]" +
            (debug_log ? " [" + getThreadId() + "]" : "") +
            (trace_log ? getTraceFormat(code_position) : " ")
        };

        return header + message;
    }

    std::string Logger::getTimeStamp() const { //TODO перенести реализацию в утилиты currentTimeFormated()  16.01
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

    std::string Logger::getThreadId() const {
        //TODO: DOESNT COMPILE ON GCC (LINUX)!!!!!!!!!!!!!!!!! // use -pthread linking option
//        const auto thread_id_max_length = 5;
//        std::string thread_id = (std::stringstream() << std::this_thread::get_id()).str();
//        thread_id.insert(0, thread_id_max_length - thread_id.length(), '0');
//        return thread_id;
        return "0000";
    }

    std::string Logger::getTraceFormat(const std::string& code_position) const {
        const auto message_offset = 34;
        std::string code_pos_word = "  Code position:";
        code_pos_word.resize(message_offset, ' ');
        std::string message_word = "  Message:";
        message_word.resize(message_offset, ' ');
        return "\n" + code_pos_word + shortenCodePosition(code_position) + "\n" + message_word;
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

        static_log("FFmpeg", convert_log_level(level), line);
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

    std::string Logger::encodeLogLevel(LogLevel value) const {
        switch (value) {
        case LogLevel::Info:
            return "info";
        case LogLevel::Warning:
            return "warn";
        case LogLevel::Error:
            return "err ";
        case LogLevel::Verbose:
        case LogLevel::Debug:
            return "deb ";
        case LogLevel::Trace:
            return "trac";
        case LogLevel::Quiet:
            return "   ";
        }
        return "?";
    }

    std::string Logger::shortenCodePosition(const std::string& value) const {
        /* fpp::TemplateProcessor<class fpp::Packet,class fpp::Packet>::sendOutputData::<lambda_c571b16bdc37e6ce04bdf51947bc2df9>::operator () */
        /* fpp::TemplateProcessor<T1,T2>::sendOutputData::<lambda_df9>::operator () */
        std::string result = value;
        { /* shortening lambda */
            const std::string keyword_lambda = "lambda_";
            const int visible_part_of_address = 3;
            const int lambda_address_lingth = 32;
            if (size_t pos = value.find(keyword_lambda); pos != std::string::npos) {
                result.erase(pos + keyword_lambda.length(), lambda_address_lingth - visible_part_of_address);
            }
        }
        { /* shortening template arguments */
//            int template_count = 0;
//            const std::string keyword_class = "class";
//            const std::string keyword_comma = ",";
        }
        return result;
    }

    Logger& Logger::instance() {
        static Logger _logger;
        return _logger;
    }

    void Logger::setLogLevel(LogLevel log_level) {
        _log_level = log_level;
    }

    void Logger::setFFmpegLogLevel(LogLevel log_level) {
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
        case LogLevel::Debug:
            ::av_log_set_level(AV_LOG_DEBUG);
            break;
        case LogLevel::Trace:
            ::av_log_set_level(AV_LOG_TRACE);
            break;
        case LogLevel::Quiet:
            ::av_log_set_level(AV_LOG_QUIET);
            break;
        }
    }

    bool Logger::ignoreMessage(LogLevel message_log_level) const {
        return (message_log_level == LogLevel::Quiet) || (message_log_level > _log_level);
    }

    void Logger::print(const std::string& caller_name, const std::string& code_position, const LogLevel log_level, const std::string& message) const {
        const auto formated_message {
            formatMessage(caller_name, code_position, log_level, message)
        };
        print(log_level, formated_message);
    }

    Logger::ConsoleHandler::ConsoleHandler(std::mutex& mutex, LogLevel log_level) :
        _h_stdout { ::GetStdHandle(STD_OUTPUT_HANDLE) }
        , _lock { mutex } {
        setConsoleColor(log_level);
    }

    Logger::ConsoleHandler::~ConsoleHandler() {
        resetConsoleColor();
    }

    void Logger::ConsoleHandler::setConsoleColor(LogLevel log_level) const {
        #ifdef _WIN32

        constexpr auto white    { FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE       };
        constexpr auto yellow   { FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY  };
        constexpr auto red      { FOREGROUND_RED|FOREGROUND_INTENSITY                   };
        constexpr auto blue     { FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY };
        constexpr auto purpule  { FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY   };

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
            case LogLevel::Debug:
                ::SetConsoleTextAttribute(_h_stdout, blue);
                return;
            case LogLevel::Trace:
                ::SetConsoleTextAttribute(_h_stdout, purpule);
                return;
        }

        #elif __linux__
            // https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal // TODO: 23.04
        #endif
    }

    void Logger::ConsoleHandler::resetConsoleColor() const {
        #ifdef _WIN32
        constexpr auto white { FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE };
        ::SetConsoleTextAttribute(_h_stdout, white);
        #elif __linux__
            // https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal // TODO: 23.04
        #endif
    }


} // namespace fpp
