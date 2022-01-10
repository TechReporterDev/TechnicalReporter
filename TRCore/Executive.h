#pragma once
#include "RoutineRunner.h"
namespace TR { namespace Core {

struct Executive
{
    Executive(std::shared_ptr<Executor> control, std::shared_ptr<Executor> processing, std::shared_ptr<Executor> io);
    
    template<class Functor>
    typename std::result_of<Functor()>::type exec(Functor& functor);

    template<class Functor>
    void async(Functor&& functor, time_t when = INVALID_TIME);

    template<class Routine>
    void run_processing(Routine&& rtn, time_t when = INVALID_TIME);

    template<class Routine>
    void run_io(int strand, Routine&& rtn); 

    std::shared_ptr<Executor> m_control;
    std::shared_ptr<Executor> m_processing;
    std::shared_ptr<Executor> m_io;
    std::shared_ptr<SerialExecutors<int>> m_strand_io;
};

std::unique_ptr<Executive> branch(const Executive& executive);
void stop(const Executive& executive);

//////////////////
// Implementation
//////////////////

template<class Functor>
typename std::result_of<Functor()>::type Executive::exec(Functor& functor)
{
    return exec_now(*m_control, [&](){
        return functor();
    }).get();
}

template<class Functor>
void Executive::async(Functor&& functor, time_t when)
{
    exec_later(*m_control, std::forward<Functor>(functor), when);
}

template<class Routine>
void Executive::run_processing(Routine&& rtn, time_t when)
{
    run({ *m_control, *m_processing },
        std::forward<Routine>(rtn), when);
}

template<class Routine>
void Executive::run_io(int strand, Routine&& rtn)
{
    run_now({ *m_control, m_strand_io->get_executor(strand) },
        std::forward<Routine>(rtn));
}

}}//namespace TR { namespace Core {