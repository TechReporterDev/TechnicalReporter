#pragma once
#include "Executor.h"
#include "STLTools\functor.h"
namespace TR {namespace Core {

template<class Functor>
struct ReturnsVoid
{
    static const bool value = std::is_same<typename std::result_of<Functor()>::type, void>::value;
};

template<class Functor>
static bool is_empty(Functor& functor)
{
    return false;
}

template<class T>
static bool is_empty(std::function<T>& functor)
{
    return static_cast<bool>(functor) == false;
}

struct RoutineRunner
{
    Executor& m_control;
    Executor& m_operating;
};

template<class Routine, class... Args>
void run(RoutineRunner routine_runner, Routine&& rtn, time_t when, Args&&... args);

template<class Routine, class... Args>
void run_now(RoutineRunner routine_runner, Routine&& rtn, Args&&... args);


//////////////////
// Implementation
//////////////////

template<class Routine, class... Args>
void run(RoutineRunner routine_runner, Routine&& rtn, time_t when, Args&&... args)
{
    exec_later(routine_runner.m_operating, &exec<std::decay_t<Routine>, std::decay_t<Args>...>,
        when,
        routine_runner,
        std::forward<Routine>(rtn),
        std::forward<Args>(args)...);
}

template<class Routine, class... Args>
void run_now(RoutineRunner routine_runner, Routine&& rtn, Args&&... args)
{
    run(routine_runner, std::forward<Routine>(rtn), INVALID_TIME, std::forward<Args>(args)...);
}

template<class Routine, class... Args>
void exec(RoutineRunner routine_runner, Routine&& rtn, Args&&... args) noexcept
{
    no_except([&]{
        try
        {
            auto functor = exec_now(routine_runner.m_control, [&]{
                auto functor =  rtn.prepare_functor(std::forward<Args>(args)...);
                if (is_empty(functor))
                {
                    rtn.on_cancel();
                }
                return functor;
            }).get();

            if (!is_empty(functor))
            {
                exec_functor(routine_runner, rtn, functor);
            }
        }
        catch (std::exception& err)
        {
            exec_now(routine_runner.m_control, [&]{
                rtn.on_failed(err);
            }).get();
        }
    });
}

template<class Routine, class Functor, typename std::enable_if<ReturnsVoid<Functor>::value, int>::type = 0>
void exec_functor(RoutineRunner routine_runner, const Routine& rtn, Functor& functor)
{
    functor();
    exec_now(routine_runner.m_control, [&]{
        rtn.on_completed();
    }).get();
}

template<class Routine, class Functor, typename std::enable_if<!ReturnsVoid<Functor>::value, int>::type = 0>
void exec_functor(RoutineRunner routine_runner, const Routine& rtn, Functor& functor)
{
    auto result = functor();
    exec_now(routine_runner.m_control, [&]{
        rtn.on_completed(std::move(result));
    }).get();
}

}}//namespace TR { namespace Core {