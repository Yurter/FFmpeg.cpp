#pragma once
#include <thread>
#include <functional>
#include <atomic>
#include <fpp/core/Object.hpp>

namespace fpp {

    using LoopFunction = std::function<Code(void)>;

    class Thread : public Object {

    public:

        Thread();
        Thread(LoopFunction loop_function);
        Thread& operator=(Thread&& other);
        Thread(const Thread&)               = delete;
        Thread& operator=(const Thread&)    = delete;
        virtual ~Thread() override;

        Code                start();
        Code                stop();
        bool                running() const;
        void                join();

        Code                exitCode()      const;
        std::string         exitMessage()   const;

    protected:

        virtual Code        run();
        virtual void        onStart();
        virtual void        onStop();

    private:

        std::thread         _thread;
        std::atomic_bool    _running;
        std::atomic_bool    _stop_flag;
        Code                _exit_code;
        std::string         _exit_message;
        LoopFunction        _loop_function;

    };

} // namespace fpp
