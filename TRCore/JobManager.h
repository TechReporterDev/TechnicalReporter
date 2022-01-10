#pragma once
namespace TR { namespace Core {

class Job
{
public:
    enum class State {STATE_IDLE, STATE_RUNNING, STATE_FAILED, STATE_COMPLETED};
    using CompletedHandler = std::function<void(void)>;
    using FailedHandler = std::function<void(const std::exception&)>;
    
    Job();
    virtual ~Job();
    
    int             get_id() const;
    State           get_state() const;
    
    void            run(CompletedHandler completedHandler, FailedHandler failedHandler);
    void            cancel();

protected:
    //override
    virtual void    do_run() = 0;
    virtual void    do_cancel();

    void            emit_completed();
    void            emit_failed(const std::exception& err);

    State m_state;
    CompletedHandler m_completedHandler;
    FailedHandler m_failedHandler;
    int m_id;
};

class Batch: public Job, public std::enable_shared_from_this<Batch>
{
public:
    using Container = std::vector<std::shared_ptr<Job>>;
    using Iterator = Container::iterator;
    enum Policy { CONCURRENTLY = 0, SEQUENTAL };

    Batch(Policy policy = Policy::CONCURRENTLY);
    void                add_job(std::shared_ptr<Job> job);
    Iterator            begin();
    Iterator            end();

protected:
    // Job override
    virtual void        do_run() override;
    virtual void        do_cancel() override;

    void                run_sequental();
    void                run_concurrently();
    void                on_completed();
    void                on_failed(const std::exception& err);

private:
    Container m_container;
    Policy m_policy;
};

template<class StrandID>
class JobManager
{
public:
    using CompletedHandler = std::function<void(StrandID, std::shared_ptr<Job>)>;
    using FailedHandler = std::function<void(StrandID, std::shared_ptr<Job>, const std::exception&)>;

    JobManager();
    JobManager(CompletedHandler completed_handler, FailedHandler failed_handler);

    void run_job(StrandID strand_id, std::shared_ptr<Job> job); 
    void cancel_job(int job_id);
    void clear_strand(StrandID strand_id);
    bool is_empty(StrandID strand_id);
    std::shared_ptr<Job> current_job(StrandID strand_id);

    void connect_completed(CompletedHandler completed_handler);
    void connect_failed(FailedHandler failed_handler);

private:    
    struct Strand
    {
        StrandID m_id;
        std::list<std::shared_ptr<Job>> m_queue;
        std::shared_ptr<Job> m_current;
    };

    void on_completed(Strand& strand);
    void on_failed(Strand& strand, const std::exception& err);
    void run_next(Strand& strand);

    CompletedHandler m_completed_handler;
    FailedHandler m_failed_handler;
    std::map<StrandID, Strand> m_strands;
};

////////////////
// implementation
////////////////

template<class StrandID>
JobManager<StrandID>::JobManager()
{
}

template<class StrandID>
JobManager<StrandID>::JobManager(CompletedHandler completed_handler, FailedHandler failed_handler):
    m_completed_handler(completed_handler),
    m_failed_handler(failed_handler)
{
}

template<class StrandID>
void JobManager<StrandID>::run_job(StrandID strand_id, std::shared_ptr<Job> job)
{
    auto& strand = m_strands[strand_id];
    strand.m_id = strand_id;
    strand.m_queue.push_back(job);

    if (!strand.m_current)
    {
        run_next(strand);   
    }
}

template<class StrandID>
void JobManager<StrandID>::cancel_job(int job_id)
{
    for (auto& strand : m_strands | boost::adaptors::map_values)
    {
        if (strand.m_current && strand.m_current->get_id() == job_id)
        {
            strand.m_current->cancel();
            strand.m_current = nullptr;

            if (!strand.m_queue.empty())
            {
                run_next(strand);
            }
            return;
        }

        for (auto position = strand.m_queue.begin(); position != strand.m_queue.end(); ++position)
        {
            if ((*position)->get_id() == job_id)
            {
                strand.m_queue.erase(position);
                return;
            }
        }       
    }
    _ASSERT(false); // job already done, or logic error?
}

template<class StrandID>
void JobManager<StrandID>::clear_strand(StrandID strand_id)
{
    auto position = m_strands.find(strand_id);
    if (position == m_strands.end())
    {
        return;
    }
    
    auto& strand = position->second;
    if (strand.m_current)
    {
        strand.m_current->cancel();
        strand.m_current = nullptr;     
    }
    strand.m_queue.clear();
}

template<class StrandID>
bool JobManager<StrandID>::is_empty(StrandID strand_id)
{
    return current_job(strand_id) == nullptr;
}

template<class StrandID>
std::shared_ptr<Job> JobManager<StrandID>::current_job(StrandID strand_id)
{
    auto position = m_strands.find(strand_id);
    if (position == m_strands.end())
    {
        return nullptr;
    }

    auto& strand = position->second;
    return strand.m_current;
}

template<class StrandID>
void JobManager<StrandID>::connect_completed(CompletedHandler completed_handler)
{
    _ASSERT(!m_completed_handler);
    m_completed_handler = completed_handler;
}

template<class StrandID>
void JobManager<StrandID>::connect_failed(FailedHandler failed_handler)
{
    _ASSERT(!m_failed_handler);
    m_failed_handler = failed_handler;
}

template<class StrandID>
void JobManager<StrandID>::on_completed(Strand& strand)
{
    _ASSERT(strand.m_current && strand.m_current->get_state() == Job::State::STATE_COMPLETED);

    if (m_completed_handler)
    {
        m_completed_handler(strand.m_id, strand.m_current);
    }

    strand.m_current = nullptr;
    if (!strand.m_queue.empty())
    {
        run_next(strand);
    }
}

template<class StrandID>
void JobManager<StrandID>::on_failed(Strand& strand, const std::exception& err)
{
    _ASSERT(strand.m_current && strand.m_current->get_state() == Job::State::STATE_FAILED);

    if (m_failed_handler)
    {
        m_failed_handler(strand.m_id, strand.m_current, err);
    }

    strand.m_current = nullptr;
    if (!strand.m_queue.empty())
    {
        run_next(strand);
    }
}

template<class StrandID>
void JobManager<StrandID>::run_next(Strand& strand)
{
    _ASSERT(!strand.m_current && !strand.m_queue.empty());

    no_except([&]{
        strand.m_current = strand.m_queue.front();
        strand.m_queue.pop_front();
        strand.m_current->run(
            [this, &strand]{
                on_completed(strand);
            },
            [this, &strand](const std::exception& err){
                on_failed(strand, err);
            }
        );
    });
}

}} //namespace TR { namespace Core {