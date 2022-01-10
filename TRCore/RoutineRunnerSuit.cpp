#include "stdafx.h"
#include "RoutineRunnerSuit.h"
#include "CoreTestAssist.h"
#include "Counters.h"
namespace {

struct Routine
{
    enum State {STATE_UNDEFINED = 0, STATE_COMPLETED, STATE_FAILED, STATE_CANCELED};

    Routine(State target_state):
        m_target_state(target_state),
        m_result(std::make_shared<Result>()),
        m_state(std::make_shared<State>(STATE_UNDEFINED))
    {
    }

    Routine(const Routine&) = default;
    Routine(Routine&& r):
        m_arg(std::move(r.m_arg)),
        m_target_state(r.m_target_state),
        m_result(r.m_result),
        m_state(r.m_state)
    {
    }

    Routine& operator = (const Routine&) = default;
    Routine& operator = (Routine&& r)
    {
        m_arg = std::move(r.m_arg);
        m_target_state = r.m_target_state;
        m_result = r.m_result;
        m_state = r.m_state;
        return *this;
    }

    std::function<Result(void)> prepare_functor()
    {
        switch (m_target_state)
        {
        case STATE_COMPLETED:
            return []{
                return Result(1);
            };
        
        case STATE_FAILED: 
            throw std::logic_error("");

        case STATE_CANCELED:
            return nullptr;

        default:
            _ASSERT(false);
        }

        _ASSERT(false);
        return nullptr;
    }

    void on_completed(Result result) const
    {
        *m_result = std::move(result);
        *m_state = STATE_COMPLETED;
    }

    void on_failed(const std::exception&) const
    {
        *m_state = STATE_FAILED;
    }

    void on_cancel()
    {
        *m_state = STATE_CANCELED;
    }

    Arg m_arg;
    State m_target_state;
    std::shared_ptr<State> m_state;
    std::shared_ptr<Result> m_result;   
};

void concurrent_executor_test()
{
    std::atomic<int> result = 0;
    auto increment = [&result](){ std::this_thread::sleep_for(std::chrono::milliseconds(100));  ++result; };
    auto pool_size = std::thread::hardware_concurrency();
    ConcurrentExecutor concurrent_executor(pool_size);
    for (int i = 0; i < 100; ++i)
    {
        concurrent_executor.exec(increment);
    }

    std::chrono::milliseconds estimate_time = std::chrono::milliseconds(10000 / pool_size);

    std::this_thread::sleep_for(estimate_time / 2);
    TEST_ASSERT(result > 40 && result < 60);

    std::this_thread::sleep_for(estimate_time / 2);
    TEST_ASSERT(result > 90);

    std::this_thread::sleep_for(estimate_time / 2);
    TEST_ASSERT(result = 100);
}

void routine_complete_test()
{
    ResetCounters();
    Routine rtn(Routine::STATE_COMPLETED);
     
    DirectExecutor control;
    DirectExecutor processing;
    RoutineRunner routine_runner { control, processing };
    run_now(routine_runner, rtn);

    TEST_ASSERT(*rtn.m_state == Routine::STATE_COMPLETED);
    TEST_ASSERT(*rtn.m_result == Result(1));

    TEST_ASSERT(Arg::copy_counter == 1);
    TEST_ASSERT(Arg::move_counter > 0);
    TEST_ASSERT(Result::copy_counter == 0);
    TEST_ASSERT(Result::move_counter >= 2);
}

void routine_failed_test()
{
    ResetCounters();
    Routine rtn(Routine::STATE_FAILED);
     
    DirectExecutor control;
    DirectExecutor processing;
    RoutineRunner routine_runner{ control, processing };
    run_now(routine_runner, rtn);

    TEST_ASSERT(*rtn.m_state == Routine::STATE_FAILED);
    TEST_ASSERT(*rtn.m_result == Result(0));

    TEST_ASSERT(Arg::copy_counter == 1);
    TEST_ASSERT(Arg::move_counter > 0);
    TEST_ASSERT(Result::copy_counter == 0);
    TEST_ASSERT(Result::move_counter >= 0);
}

void routine_canceled_test()
{
    ResetCounters();
    Routine rtn(Routine::STATE_CANCELED);
     
    DirectExecutor control;
    DirectExecutor processing;
    RoutineRunner routine_runner{ control, processing };
    run_now(routine_runner, rtn);

    TEST_ASSERT(*rtn.m_state == Routine::STATE_CANCELED);
    TEST_ASSERT(*rtn.m_result == Result(0));

    TEST_ASSERT(Arg::copy_counter == 1);
    TEST_ASSERT(Arg::move_counter > 0);
    TEST_ASSERT(Result::copy_counter == 0);
    TEST_ASSERT(Result::move_counter >= 0);
}

}//namespace {

test_assist::test_suit get_routine_runner_suit()
{
    test_assist::test_suit ts(L"routine_runner_suit");
    ts
        //.add({ L"concurrent_executor_test", &concurrent_executor_test })
        .add({L"routine_complete_test", &routine_complete_test})
        .add({L"routine_failed_test", &routine_failed_test})
        .add({L"routine_canceled_test", &routine_canceled_test});
        
    return ts;
}