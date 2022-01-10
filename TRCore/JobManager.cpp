#include "stdafx.h"
#include "JobManager.h"
#include "CoreImpl.h"
namespace TR { namespace Core {

static int gen_job_id()
{
    static int s_id = 0;
    return ++s_id;
}

Job::Job():
    m_state(State::STATE_IDLE),
    m_id(gen_job_id())
{
}

Job::~Job()
{
    _ASSERT(m_state != State::STATE_RUNNING);
}

int Job::get_id() const
{
    return m_id;
}

Job::State Job::get_state() const
{
    return m_state;
}

void Job::run(CompletedHandler completedHandler, FailedHandler failedHandler)
{
    _ASSERT(m_state == State::STATE_IDLE);
    m_completedHandler = std::move(completedHandler);
    m_failedHandler = std::move(failedHandler);
    m_state = State::STATE_RUNNING;
    
    try
    {
        do_run();
    }
    catch (std::exception& err)
    {
        emit_failed(err);
    }
}

void Job::cancel()
{
    _ASSERT(m_state == State::STATE_RUNNING);
    do_cancel();
    m_state = State::STATE_IDLE;
    m_completedHandler = nullptr;
    m_failedHandler = nullptr;
}

void Job::do_cancel()
{
}

void Job::emit_completed()
{
    no_except([&]{
        _ASSERT(m_state == State::STATE_RUNNING);
        m_state = State::STATE_COMPLETED;
        if (m_completedHandler)
        {
            m_completedHandler();
        }
    }); 
}

void Job::emit_failed(const std::exception& err)
{
    no_except([&]{
        _ASSERT(m_state == State::STATE_RUNNING);
        m_state = State::STATE_FAILED;
        if (m_failedHandler)
        {
            m_failedHandler(err);
        }
    });
}

Batch::Batch(Policy policy):
    m_policy(policy)
{
}

void Batch::add_job(std::shared_ptr<Job> job)
{
    m_container.push_back(job);
}

Batch::Iterator Batch::begin()
{
    return m_container.begin();
}

Batch::Iterator Batch::end()
{
    return m_container.end();
}

void Batch::do_run()
{
    try
    {
        if (m_container.empty())
        {
            emit_completed();
            return;
        }

        // Batch can be finished and deleted before it runs all of his subjobs,
        // so we must lock it, untill do_run finish
        auto shared_this = shared_from_this();      
        _ASSERT(shared_this);       

        switch (m_policy)
        {
        case Policy::CONCURRENTLY:
            run_concurrently();
            break;

        case Policy::SEQUENTAL:
            run_sequental();
            break;

        default:
            _ASSERT(false);
        }
    }
    catch (std::exception&)
    {
        do_cancel();
        throw;
    }
}

void Batch::do_cancel()
{
    for (auto job : m_container)
    {
        if (job->get_state() == State::STATE_RUNNING)
        {
            job->cancel();
        }
    }
}

void Batch::run_sequental()
{
    auto run_next = [this](auto& next, size_t position){
        if (m_container.size() <= position)
        {
            on_completed();
            return;
        }

        try
        {
            m_container[position]->run(
                [this, next, position] {
                    next(next, position +1);
                },
                [this](const std::exception& err) {
                    on_failed(err);
                }
            );
        }
        catch (std::exception& err)
        {
            auto shared_this = shared_from_this();
            on_failed(err);
        }
    };

    run_next(run_next, 0);
}

void Batch::run_concurrently()
{
    for (auto job : m_container)
    {
        if (m_state != State::STATE_RUNNING)
        {
            break;
        }

        job->run(
            [this] {
                on_completed();
            },
            [this](const std::exception& err) {
                on_failed(err);
            }
        );
    }
}

void Batch::on_completed()
{
    // prevent from deletion inside handler
    auto shared_this = shared_from_this();

    for (auto job : m_container)
    {
        if (job->get_state() != State::STATE_COMPLETED)
        {
            return;
        }
    }
    emit_completed();
}

void Batch::on_failed(const std::exception& err)
{
    // prevent from deletion inside handler
    auto shared_this = shared_from_this();

    do_cancel();
    emit_failed(err);
}

}} //namespace TR { namespace Core {