#include "stdafx.h"
#include "Core.h"
#include "CoreImpl.h"
#include "Features.h"
#include "PlainFacade.h"
#include "Async.h"
#include "EventFlow.h"
#include "Transaction.h"
#include "Mail.h"
namespace TR { namespace Core {

void install(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema)
{
    install_database(user, passwd, host, schema);
    auto db = connect_database(user, passwd, host, schema);
    Transaction t(*db);
    odb::core::schema_catalog::create_schema(*db);
    Roles::install(*db);
    Sources::install(*db);
    Security::install(*db);
    Scheduler::install(*db);
    t.commit();
}

void uninstall(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema)
{
    uninstall_database(user, passwd, host, schema);
}

bool verify(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema)
{
    return verify_database(user, passwd, host, schema);
}

Core::Core():
    m_features(std::make_unique<Features>()),
    m_event_dispatchers(std::make_unique<EventDispatchers>()),
    m_control(std::make_shared<DirectExecutor>())
{
}

Core::Core(Core&& core):
    m_core_impl(std::move(core.m_core_impl)),
    m_features(std::move(core.m_features)),
    m_event_dispatchers(std::move(core.m_event_dispatchers)),
    m_frontend(std::move(core.m_frontend)),
    m_async(std::move(core.m_async)),
    m_event_flow(std::move(core.m_event_flow)),
    m_control(core.m_control)
{
}

Core::~Core()
{   
    if (running())
    {
        m_async->stop();
        m_core_impl->stop();
        m_control->stop();
    }
}

void Core::connect_database(std::unique_ptr<Database> db)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (connected())
    {
        throw Exception(L"The core is already connected");
    }

    auto core_impl = std::make_unique<CoreImpl>(std::move(db));
    auto async = std::make_unique<Async>(*core_impl);
    auto event_flow = std::make_unique<EventFlow>(*core_impl);
    event_flow->set_dispatchers(m_event_dispatchers.get());
    event_flow->set_async(async.get());
    
    m_core_impl = std::move(core_impl);
    m_async = std::move(async);
    m_event_flow = std::move(event_flow);
}

void Core::connect_mysql(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema)
{
    connect_database(TR::Core::connect_database(user, passwd, host, schema));
}

bool Core::is_connected() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return connected();
}

void Core::connect_event_dispatcher(std::unique_ptr<EventDispatcher> event_disp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    _ASSERT(!running());
    exec_now(*m_control, [&]{
        m_event_dispatchers->push_back(std::move(event_disp));
    }).get();   
}

void Core::disconnect_event_dispatcher(EventDispatcher* dispatcher)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    _ASSERT(!running());

    exec_now(*m_control, [&]{
        for (auto position = m_event_dispatchers->begin(); position != m_event_dispatchers->end(); ++position)
        {
            if (position->get() == dispatcher)
            {
                m_event_dispatchers->erase(position);
                return;
            }
        }
        throw Exception(L"Dipatcher not found");
    }).get();   
}

EventDispatcher* Core::find_event_dispatcher(std::function<bool(EventDispatcher&)> pred) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return exec_now(*m_control, [&]{
        auto found = boost::find_if(*m_event_dispatchers | boost::adaptors::indirected, pred);
        if (found.base() == m_event_dispatchers->end())
        {
            return (EventDispatcher*)nullptr;       
        }
        return &*found;
    }).get();
}

void Core::add_feature(std::unique_ptr<Feature> feature)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    exec_now(*m_control, [&]{
        m_features->add_feature(std::move(feature));
    }).get();
}

bool Core::has_feature(UUID feature_uuid) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    return exec_now(*m_control, [&]{
        return m_features->find_feature(feature_uuid) != nullptr;
    }).get();
}

void Core::remove_feature(UUID feature_uuid)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    exec_now(*m_control, [&]{
        m_features->remove_feature(feature_uuid);
    }).get();
}

void Core::setup_mailing(SMTP smtp, std::wstring email_from)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    exec_now(*m_control, [&]{
        m_core_impl->m_mailing.setup(std::move(smtp), std::move(email_from));
    }).get();
}

void Core::run()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!connected())
    {
        throw Exception(L"Call 'connect' before");
    }

    if (running())
    {
        throw Exception(L"The core is already running");
    }

    Transaction t(*m_core_impl->m_db);
    m_features->restore_installed(m_core_impl.get(), t);

    transact_assign(m_control, std::make_unique<ConcurrentExecutor>(1), t);
    m_core_impl->run(std::make_unique<Executive>(
        std::make_unique<ExpandingExecutor>(*m_control),
        std::make_unique<ConcurrentExecutor>(10),
        std::make_unique<ConcurrentExecutor>(20)
    ), t);

    m_async->run(std::make_unique<Executive>(
        std::make_unique<ExpandingExecutor>(*m_control),
        std::make_unique<ConcurrentExecutor>(10),
        std::make_unique<ConcurrentExecutor>(20)
    ), t);

    m_features->run(m_core_impl->m_executive.get(), t);

    // create frontend
    transact_assign(m_frontend, std::make_unique<Frontend>(*m_core_impl), t);
    m_frontend->set_features(m_features.get());
    m_frontend->set_async(m_async.get());

    t.commit();
}

bool Core::is_running() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return running();
}

Frontend& Core::get_frontend() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!running())
    {
        throw Exception(L"Call 'run' before");
    }

    _ASSERT(m_frontend);
    return *m_frontend;
}

bool Core::connected() const
{
    return m_core_impl != nullptr;
}

bool Core::running() const
{
    if (!connected())
    {
        return false;
    }
    return m_core_impl->m_executive != nullptr;
}

}} //namespace TR { namespace Core {