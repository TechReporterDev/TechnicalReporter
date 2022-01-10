#include "stdafx.h"
#include "Executor.h"
namespace TR { namespace Core {

void DirectExecutor::exec(Task task, time_t start_time)
{
    if (start_time != INVALID_TIME)
    {
        m_tasks.insert(std::make_pair(start_time, std::move(task)));
        return;
    }
    task();
}

void DirectExecutor::stop()
{
}

void DirectExecutor::check_timeout(time_t check_time)
{
    auto end = m_tasks.end();
    if (check_time != INVALID_TIME)
    {
        end = m_tasks.upper_bound(check_time);
    }

    std::vector<Task> tasks;
    for (auto position = m_tasks.begin(); position != end; m_tasks.erase(position++))
    {
        tasks.push_back(std::move(position->second));
    }

    for (auto& task : tasks)
    {
        task();
    }
}

class SerialExecutor::Impl: public std::enable_shared_from_this<Impl>
{
public:
    Impl(Executor& executor) :
        m_executor(executor),
        m_stop(false)
    {
    }

    ~Impl()
    {
        m_destroy_promise.set_value();
    }
    
    void exec(Task task, time_t start_time)
    {
        _ASSERT(!m_stop);

        if (start_time != INVALID_TIME)
        {
            m_executor.exec([weak_this = weak_from_this(), task = std::move(task)]() mutable {
                if (auto shared_this = weak_this.lock())
                {
                        shared_this->exec(std::move(task), INVALID_TIME);
                }
            }, start_time);

            return;
        }

        std::unique_lock<std::mutex> lock(m_mutex);
        bool first_task = m_queue.empty();
        m_queue.push(std::move(task));
        lock.unlock();

        if (first_task)
        {
            no_except([&] {
                m_executor.exec([weak_this = weak_from_this()]{
                    if (auto shared_this = weak_this.lock())
                    {
                        shared_this->exec_next();
                    }                    
                });
            });
        }
    }

    void set_stop_flag()
    {
        m_stop.store(true);
    }

    static void destroy(std::shared_ptr<Impl> impl)
    {        
        auto destroy_future = impl->m_destroy_promise.get_future();
        impl->set_stop_flag();
        impl.reset();
        destroy_future.wait();
    }

private:
    void exec_next()
    {
        no_except([&] {
            if (m_stop)
            {
                return;
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            auto task = std::move(m_queue.front());
            lock.unlock();

            task();

            lock.lock();
            m_queue.pop();
            auto no_task = m_queue.empty();
            lock.unlock();

            if (!no_task)
            {
                m_executor.exec([weak_this = weak_from_this()]{
                    if (auto shared_this = weak_this.lock())
                    {
                        shared_this->exec_next();
                    }
                });
            }
        });
    }

    Executor& m_executor;

    std::mutex m_mutex;
    std::queue<Task> m_queue;   
    std::promise<void> m_destroy_promise;
    std::atomic<bool> m_stop;
};

SerialExecutor::SerialExecutor(Executor& executor):
    m_impl(std::make_shared<Impl>(executor))
{
}

SerialExecutor::~SerialExecutor()
{
    if (m_impl)
    {
        m_impl->set_stop_flag();
    }    
}

void SerialExecutor::exec(Task task, time_t start_time)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_impl)
    {
        return;
    }
    auto impl = m_impl;
    lock.unlock();

    impl->exec(std::move(task), start_time);
}

void SerialExecutor::stop()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    _ASSERT(m_impl);
    auto impl = std::move(m_impl);
    lock.unlock();

    Impl::destroy(std::move(impl));
}

class ExpandingExecutor::Impl : public std::enable_shared_from_this<Impl>
{
public:
    Impl(Executor& executor) :
        m_executor(executor),
        m_stop(false)
    {
    }

    ~Impl()
    {
        m_destroy_promise.set_value();
    }

    void exec(Task task, time_t start_time)
    {
        _ASSERT(!m_stop);

        if (start_time != INVALID_TIME)
        {
            m_executor.exec([weak_this = weak_from_this(), task = std::move(task)]() mutable {
                if (auto shared_this = weak_this.lock())
                {
                    shared_this->exec(std::move(task), INVALID_TIME);
                }
            }, start_time);

            return;
        }

        std::unique_lock<std::mutex> lock(m_mutex);
        bool first_task = m_queue.empty();
        m_queue.push(std::move(task));
        lock.unlock();

        if (first_task)
        {
            no_except([&] {
                m_executor.exec([weak_this = weak_from_this()]{
                    if (auto shared_this = weak_this.lock())
                    {
                        shared_this->exec_next();
                    }
                });
            });
        }
    }

    void set_stop_flag()
    {
        m_stop.store(true);
    }

    static void destroy(std::shared_ptr<Impl> impl)
    {
        _ASSERT(impl);
        auto destroy_future = impl->m_destroy_promise.get_future();
        impl->set_stop_flag();
        impl.reset();       
        destroy_future.wait();
    }

private:
    void exec_next()
    {
        no_except([&] {            
            if (m_stop)
            {
                return;
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            auto task = std::move(m_queue.front());
            m_queue.pop();
            auto no_task = m_queue.empty();
            lock.unlock();          

            if (!no_task)
            {
                m_executor.exec([weak_this = weak_from_this()]{
                    if (auto shared_this = weak_this.lock())
                    {
                        shared_this->exec_next();
                    }
                });
            }

            task();
        });
    }

    Executor& m_executor;

    std::mutex m_mutex;
    std::queue<Task> m_queue;
    std::promise<void> m_destroy_promise;
    std::atomic<bool> m_stop;
};

ExpandingExecutor::ExpandingExecutor(Executor& executor) :
    m_impl(std::make_shared<Impl>(executor))
{
}

ExpandingExecutor::~ExpandingExecutor()
{
    if (m_impl)
    {
        m_impl->set_stop_flag();
    }
}

void ExpandingExecutor::exec(Task task, time_t start_time)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_impl)
    {
        return;
    }
    auto impl = m_impl;
    lock.unlock();

    impl->exec(std::move(task), start_time);
}

void ExpandingExecutor::stop()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    _ASSERT(m_impl);
    auto impl = std::move(m_impl);
    lock.unlock();

    Impl::destroy(std::move(impl));
}

class ExpandingExecutor2::Impl : public std::enable_shared_from_this<Impl>
{
public:
    Impl(std::vector<Executor*> executors) :
        m_executors(std::move(executors)),
        m_stop(false)
    {
    }

    ~Impl()
    {
        m_destroy_promise.set_value();
    }

    void exec(Task task, time_t start_time)
    {
        if (start_time != INVALID_TIME)
        {
            m_executors.front()->exec([weak_this = weak_from_this(), task = std::move(task)]() mutable {
                if (auto shared_this = weak_this.lock())
                {
                    shared_this->exec(std::move(task), INVALID_TIME);
                }
            }, start_time);

            return;
        }

        std::unique_lock<std::mutex> lock(m_mutex);
        bool first_task = m_queue.empty();
        m_queue.push(std::move(task));
        lock.unlock();

        if (first_task)
        {
            no_except([&] {
                m_executors.front()->exec([weak_this = weak_from_this()]{
                    if (auto shared_this = weak_this.lock())
                    {
                        if (shared_this->m_stop)
                        {
                            return;
                        }

                        shared_this->exec_next(0);
                    }
                });
            });
        }
    }

    void set_stop_flag()
    {
        m_stop.store(true);
    }

    static void destroy(std::shared_ptr<Impl> impl)
    {
        auto destroy_future = impl->m_destroy_promise.get_future();
        impl->set_stop_flag();
        impl.reset();
        destroy_future.wait();
    }

private:
    void exec_next(int executor_index)
    {
        no_except([&] {
            if (m_stop)
            {
                return;
            }

            auto next_executor_index = executor_index + 1;
            if (next_executor_index < m_executors.size())
            {
                m_executors[next_executor_index]->exec([weak_this = weak_from_this(), next_executor_index]{
                    if (auto shared_this = weak_this.lock())
                    {
                        shared_this->exec_next(next_executor_index);
                    }
                });
                return;
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            auto task = std::move(m_queue.front());
            m_queue.pop();
            auto no_task = m_queue.empty();
            lock.unlock();

            if (!no_task)
            {
                m_executors.front()->exec([weak_this = weak_from_this()]{
                    if (auto shared_this = weak_this.lock())
                    {
                        shared_this->exec_next(0);
                    }
                });
            }

            task();
        });
    }

    std::vector<Executor*> m_executors;

    std::mutex m_mutex;
    std::queue<Task> m_queue;
    std::promise<void> m_destroy_promise;
    std::atomic<bool> m_stop;
};

ExpandingExecutor2::ExpandingExecutor2(std::vector<Executor*> executors) :
    m_impl(std::make_shared<Impl>(std::move(executors)))
{
}

ExpandingExecutor2::~ExpandingExecutor2()
{
    if (m_impl)
    {
        m_impl->set_stop_flag();
    }
}

void ExpandingExecutor2::exec(Task task, time_t start_time)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_impl)
    {
        return;
    }
    auto impl = m_impl;
    lock.unlock();

    impl->exec(std::move(task), start_time);
}

void ExpandingExecutor2::stop()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto impl = std::move(m_impl);
    lock.unlock();

    Impl::destroy(std::move(impl));
}

ConcurrentExecutor::ConcurrentExecutor(int pool_size):
    m_pool_size(pool_size),
    m_work(m_io_service)
{
    for (int i = 0; i < m_pool_size; ++i)
    {
        m_threads.emplace_back([this](){
            m_io_service.run();
        });
    }
}

ConcurrentExecutor::~ConcurrentExecutor()
{
    if (!m_io_service.stopped())
    {
        stop();
    }
}

void ConcurrentExecutor::exec(Task task, time_t start_time)
{
    if (start_time != INVALID_TIME)
    {
        auto timer = std::make_shared<boost::asio::deadline_timer>(
            m_io_service,
            boost::posix_time::seconds(long(start_time - time(nullptr)))
            );
     
        timer->async_wait([timer, task](const boost::system::error_code& e){
            task();
        });

        return;
    }

    m_io_service.post(task);
}

void ConcurrentExecutor::stop()
{   
    _ASSERT(!m_io_service.stopped());

    m_io_service.stop();
    for (auto& thread : m_threads)
    {
        thread.join();
    }
}

}}//namespace TR { namespace Core {