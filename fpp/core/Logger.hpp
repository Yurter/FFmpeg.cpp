#pragma once
#include <fpp/core/Object.hpp>
#include <fstream>
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
        /* Сообщения, используемые при отладке кода */
        Debug,
        /* ? */
        Verbose, //TODO вынести часть лога в этот раздел из дебага
        /* Чрезвычайно подробный лог, полезный при разработке fpp */
        Trace,
    };

    class Logger : public Object {

    public:

        static Logger&      instance(std::string log_dir = "fpp_log");

        void                setLogLevel(LogLevel log_level);
        void                setFFmpegLogLevel(LogLevel log_level);

        bool                ignoreMessage(const LogLevel message_log_level);
        void                print(const std::string& caller_name, const std::string& code_position, const LogLevel log_level, const std::string& message);

    private:
                                        // TODO дирректория создается безусловно 03.02
        Logger(std::string log_dir);    // TODO fix log dir 23.01
        virtual ~Logger() override;

        Logger(Logger const&)               = delete;
        Logger(Logger const&&)              = delete;
        Logger& operator=(Logger const&)    = delete;

    private:

        Code                print(const LogLevel log_level, const std::string& log_text);
        void                openFile(const std::string& log_dir);
        void                closeFile();
        std::string         genFileName() const;
        std::string         formatMessage(std::string caller_name, const std::string& code_position, LogLevel log_level, const std::string& message);
        std::string         getTimeStamp() const;
        std::string         getThreadId() const;
        std::string         getTraceFormat(const std::string& code_position) const;
        static void         log_callback(void* ptr, int level, const char* fmt, va_list vl);
        static LogLevel     convert_log_level(int ffmpeg_level);
        std::string         encodeLogLevel(LogLevel value);
        std::string         shortenCodePosition(const std::string& value) const;

    private:

        LogLevel            _log_level;
        std::ofstream       _file;
        std::mutex          _print_mutex;

    };

} // namespace fpp

/* Обертка пространства имён fpp */
#define FPP_BEGIN do { using namespace fpp;
#define FPP_END } while (false)

/* Макрос получения экзмепляра объекта класса Logger */
#define logger fpp::Logger::instance()

/* Макрос установки дирректории, в которой находятся файлы лога.
 * Должен вызыватся первым по отношению к остальным макросам    */
#define set_log_dir(x) fpp::Logger::instance(x)

/* Макрос установки уровня лога - сообщения, имеющие урень выше установленного, игнорируются */
#define set_log_level(x)        logger.setLogLevel(x)
#define set_ffmpeg_log_level(x) logger.setFFmpegLogLevel(x)

/* Макрос для отправки строкового сообщения в лог */
//#define log_message(caller_name, log_level, message)    FPP_BEGIN\
//                                                            if constexpr (!logger.ignoreMessage(log_level)) {\
//                                                                std::stringstream log_ss;\
//                                                                log_ss << message;\
//                                                                logger.print(caller_name, code_pos, log_level, log_ss.str());\
//                                                            }\
//                                                        FPP_END
#define log_message(caller_name, log_level, message) FPP_BEGIN\
    if_not(logger.ignoreMessage(log_level)) {\
        std::stringstream log_ss;\
        log_ss << message;\
        logger.print(caller_name, CODE_POS, log_level, log_ss.str());\
    }\
    FPP_END

/* Макросы для отправки строкоых сообщений в лог */
#define log_info(message)       log_message(this->name(), LogLevel::Info,       message)
#define log_warning(message)    log_message(this->name(), LogLevel::Warning,    message)
#define log_error(message)      log_message(this->name(), LogLevel::Error,      message)
#define log_debug(message)      log_message(this->name(), LogLevel::Debug,      message)
#define log_verbose(message)    log_message(this->name(), LogLevel::Verbose,    message)
#define log_trace(message)      log_message(this->name(), LogLevel::Trace,      message)

/* Макросы для отправки потоковых сообщений в лог вне контекста fpp */
#define static_log_info(caller_name, message)       log_message(caller_name, LogLevel::Info,    message)
#define static_log_warning(caller_name, message)    log_message(caller_name, LogLevel::Warning, message)
#define static_log_error(caller_name, message)      log_message(caller_name, LogLevel::Error,   message)
#define static_log_debug(caller_name, message)      log_message(caller_name, LogLevel::Debug,   message)
#define static_log_trace(caller_name, message)      log_message(caller_name, LogLevel::Trace,   message)
#define static_log_verbose(caller_name, message)    log_message(caller_name, LogLevel::Verbose, message)
#define static_log(caller_name, log_level, message) log_message(caller_name, log_level,         message)

/* ? */
#define CODE_POS std::string(__FUNCTION__) + ", line: " + std::to_string(__LINE__) + " "
