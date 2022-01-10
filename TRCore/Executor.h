#pragma once
#include "stddecl.h"
#include <boost\asio.hpp>
namespace TR { namespace Core {

const time_t INVALID_TIME = 0;

class Executor
{
public:
    using Task = std::function<void(void)>;
    
    virtual ~Executor() = default;

    virtual void    exec(Task task, time_t start_time = INVALID_TIME) = 0;
    virtual void    stop() = 0;
};

class DirectExecutor: public Executor
{
public:
    virtual void exec(Task task, time_t start_time = INVALID_TIME) override;
    virtual void stop() override;
    void check_timeout(time_t check_time = time(nullptr));

private:
    std::multimap<time_t, Task> m_tasks;
};

class SerialExecutor: public Executor
{
public:
    SerialExecutor(Executor& target);
    ~SerialExecutor();

    virtual void exec(Task task, time_t start_time = INVALID_TIME) override;
    virtual void stop() override;

private:
    class Impl;
    std::shared_ptr<Impl> m_impl;
    std::mutex m_mutex;
};

class ExpandingExecutor : public Executor
{
public:
    ExpandingExecutor(Executor& target);
    ~ExpandingExecutor();

    virtual void exec(Task task, time_t start_time = INVALID_TIME) override;
    virtual void stop() override;

private:
    class Impl;
    std::shared_ptr<Impl> m_impl;    
    std::mutex m_mutex;
};

class ExpandingExecutor2 : public Executor
{
public:
    ExpandingExecutor2(std::vector<Executor*> targets);
    ~ExpandingExecutor2();

    virtual void exec(Task task, time_t start_time = INVALID_TIME) override;
    virtual void stop() override;

private:
    class Impl;
    std::shared_ptr<Impl> m_impl;
    std::mutex m_mutex;
};

class ConcurrentExecutor: public Executor
{
public:
    ConcurrentExecutor(int pool_size);
    ~ConcurrentExecutor();
    virtual void exec(Task task, time_t start_time = INVALID_TIME) override;
    virtual void stop() override;

private:
    boost::asio::io_service m_io_service;
    boost::asio::io_service::work m_work;
    std::vector<std::thread> m_threads;
    int m_pool_size;
};

template<class T>
class ExecutorCollection
{
public:
    virtual Executor& get_executor(const T& key) = 0;
};

template<class T>
class SerialExecutors : public ExecutorCollection<T>
{
public:
    SerialExecutors(Executor& executor):
        m_executor(executor)
    {
    }

    virtual Executor& get_executor(const T& strand)
    {
        auto position = m_serial_executors.find(strand);
        if (position == m_serial_executors.end())
        {
            position = m_serial_executors.insert(std::make_pair(
                strand, 
                std::make_unique<SerialExecutor>(m_executor))).first;
        }
        return *position->second;
    }

private:
    Executor& m_executor;
    std::map<T, std::unique_ptr<SerialExecutor>> m_serial_executors;
};


template <class F, class... Args>
std::future<typename std::result_of<F(std::decay_t<Args>&&...)>::type> 
exec_later(Executor& e, F&& f, time_t when, Args&&... args)
{
    using R = typename std::result_of<F(std::decay_t<Args>&&...)>::type;
    auto task = std::make_shared<std::packaged_task<R()>>(
        stl_tools::force_movable(
            std::bind(
                stl_tools::force_move_args(std::forward<F>(f)), std::forward<Args>(args)...)));
    
    e.exec([task](){ (*task)(); }, when);
    return task->get_future();
}

template <class F, class... Args>
std::future<typename std::result_of<F(std::decay_t<Args>&&...)>::type> exec_now(Executor& e, F&& f, Args&&... args)
{
    return exec_later(e, std::forward<F>(f), INVALID_TIME, std::forward<Args>(args)...);
}

}}// namespace TR { namespace Core {