#include "Thread.hpp"
#include <fpp/core/Utils.hpp>
#include <fpp/core/time/Chronometer.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <fpp/core/Logger.hpp>
#include <exception>

#define TRY     try
#define CATCH   catch (const FFmpegException& e) {\
                    log_error("ffmpeg's exception: " << e.what());\
                    _exit_code = Code::EXCEPTION;\
                    _exit_message = e.what();\
                }\
                catch (const std::exception& e) {\
                    log_error("std::exception: " << e.what());\
                    _exit_code = Code::EXCEPTION;\
                    _exit_message = e.what();\
                }\
                catch (...) {\
                    log_error("unknown exception!");\
                    _exit_code = Code::EXCEPTION;\
                    _exit_message = "none";\
                }

namespace fpp {

    Thread::Thread()
        : Thread(std::bind(&Thread::run,this)) {
    }

    Thread::Thread(LoopFunction loop_function)
        : _running { false }
        , _stop_flag { false }
        , _exit_code { Code::OK }
        , _loop_function { loop_function } {
        setName("Thread");
    }

    Thread& Thread::operator=(Thread&& other) {
        _thread         = std::move(other._thread);
        _running        = other._running.load();
        _loop_function  = other._loop_function;
        return *this;
    }

    Thread::~Thread() {
        stop();
        join();
//        if (isRunning()) {
//            quit();
//            wait();
//        }
    }

    Code Thread::start() {
//        return_if(running(), Code::OK);
        _stop_flag = false;
//        if_not(inited()) { /*try_to*/(init()); }
        _running = true;
        _thread = std::thread([this]() {
            TRY {
                log_debug("Thread started");
                onStart();
                do {
                    if (_stop_flag) { break; }
                    log_trace("Execute loop function");
                    _exit_code = _loop_function();
                } while (!utils::exit_code(_exit_code));

                const std::string log_message = utils::error_code(_exit_code)
                        ? "Thread finished with error code: "
                        : "Thread correctly finished with code: ";
                log_debug(log_message << _exit_code);
            } CATCH

            TRY {
                onStop();
            } CATCH

            log_debug("Thread finished");
            _running = false;
        });
        return Code::OK;
    }

    Code Thread::stop() {
//        return_if_not(running(), Code::OK);
        log_debug("Thread stoping");
        _stop_flag = true;
        join();
        log_debug("Thread stopped");
        return Code::OK;
    }

    bool Thread::running() const {
        return _running;
    }

    void Thread::join() {
        if (_thread.joinable()) { _thread.join(); }
    }

    Code Thread::exitCode() const {
        return _exit_code;
    }

    Code Thread::run() {
        return Code::NOT_IMPLEMENTED;
    }

    void Thread::onStart() {
    }

    void Thread::onStop() {
    }

} // namespace fpp
