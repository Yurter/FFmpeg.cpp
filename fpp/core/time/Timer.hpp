#pragma once
#include <functional>
#include <thread>
#include <chrono>
#include <fpp/core/async/AsyncQueue.hpp>
#include <fpp/core/Utils.hpp>

namespace fpp {

    using TimerFunction = std::function<void()>;
    using ExecuteTime = std::chrono::time_point<std::chrono::steady_clock>;

    enum class ExecuteMode: uint8_t {
        SINGLE_SHOT,
        INTERVAL,
    };

    struct ScheduledFunction {

        ScheduledFunction() = default;
        ScheduledFunction(TimerFunction action_value
                          , int64_t delay_value
                          , ExecuteMode execute_mode_value) :
            action(action_value)
          , delay(delay_value)
          , execute_mode(execute_mode_value)
        {
            calculateExecuteTime();
        }

        ScheduledFunction(const ScheduledFunction& other) { copy(other); }
        ScheduledFunction(const ScheduledFunction&& other) { copy(other); }

        ScheduledFunction& operator=(const ScheduledFunction& other) {
            copy(other);
            return *this;
        }

        ScheduledFunction& operator=(const ScheduledFunction&& other) {
            copy(other);
            return *this;
        }

        ~ScheduledFunction() = default;

        void calculateExecuteTime() {
            execute_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
        }

        TimerFunction   action;
        int64_t         delay;
        ExecuteTime     execute_time;
        ExecuteMode     execute_mode;

    private:

        void copy(const ScheduledFunction& other) {
            action = other.action;
            delay = other.delay;
            execute_time = other.execute_time;
            execute_mode = other.execute_mode;
        }

    };

    class Timer {

    public:

        Timer() :
            _stop_flag(false)
        {
//            static_log_info("T", "CONSTR");
            _thread = std::thread([&]() {
                ScheduledFunction delayed_function;
                while (_function_queue.wait_and_pop(delayed_function)) {
                    std::this_thread::sleep_until(delayed_function.execute_time);
                    delayed_function.action();
                    if (_stop_flag) { break; }
                    if (delayed_function.execute_mode == ExecuteMode::INTERVAL) {
                        delayed_function.calculateExecuteTime();
                        if (!_function_queue.push(delayed_function)) {
                            static_log_error("T", "push failed");
                        }
                    }
                }
            });
        }

        ~Timer() {
//            static_log_info("T", "DESTR");
            _stop_flag = true;
            _function_queue.clear();
            _function_queue.stop_wait();
            if (_thread.joinable()) {
                _thread.join();
            }
        }

        void setTimeout(TimerFunction function, int delay) {
            if (!_function_queue.push({ function, delay, ExecuteMode::SINGLE_SHOT })) {
                static_log_error("T", "push failed");
            }
        }

        void setInterval(TimerFunction function, int interval) {
            int& delay = interval;
            if (!_function_queue.push({ function, delay, ExecuteMode::INTERVAL })) {
                static_log_error("T", "push failed");
            }
        }

        void reset() {
            _function_queue.clear();
        }

//        void                start(int64_t msec);
//        void                restart();
//        void                stop();
//        int64_t             interval() const;
//        bool                isActive() const;
//        bool                isSingleShot() const;
//        int64_t             remainingTime() const;
//        void                setInterval(int msec);
//        void                setSingleShot(bool singleShot);

    private:

//        void planSchedule() {
//            std::sort(std::begin(_function_queue), std::end(_function_queue), {}(){return true;});
//        }

    private:

        using FunctionsQueue = /*std::priority_queue<ScheduledFunction>;*/AsyncQueue<ScheduledFunction>;

        FunctionsQueue      _function_queue;
        std::thread         _thread;
        bool                _stop_flag;

    };


} // namespace fpp
