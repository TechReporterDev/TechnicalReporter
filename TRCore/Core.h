#pragma once
#include "BackgndFwd.h"
#include "Frontend.h"
#include "EventDispatcher.h"
#include "Mail.h"
namespace TR { namespace Core {
struct CoreImpl;
class Features;
class Feature;
class Async;
class PlainFacade;
class EventFlow;

void install(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema = default_schema);
void uninstall(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema = default_schema);
bool verify(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema = default_schema);

class Core
{
public:
    Core();
    Core(Core&& core);
    Core(const Core&) = delete;
    ~Core();

    Core& operator = (Core&& core) = delete;
    Core& operator = (const Core&) = delete;
    
    //core connect
    void                connect_database(std::unique_ptr<Database> db);
    void                connect_mysql(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema = default_schema);
    bool                is_connected() const;

    // event dispatchers
    void                connect_event_dispatcher(std::unique_ptr<EventDispatcher> event_disp);
    void                disconnect_event_dispatcher(EventDispatcher* dispatcher);
    EventDispatcher*    find_event_dispatcher(std::function<bool(EventDispatcher&)> pred) const;

    // features
    void                add_feature(std::unique_ptr<Feature> feature);
    bool                has_feature(UUID feature_uuid) const;
    void                remove_feature(UUID feature_uuid);  

    // setup mailing
    void                setup_mailing(SMTP smtp, std::wstring email_from);

    //core run
    void                run();
    bool                is_running() const;
    Frontend&           get_frontend() const;

private:
    using EventDispatchers = std::vector<std::unique_ptr<EventDispatcher>>;

    bool                connected() const;
    bool                running() const;

    std::unique_ptr<CoreImpl> m_core_impl;
    std::unique_ptr<Features> m_features;
    std::unique_ptr<EventDispatchers> m_event_dispatchers;

    std::unique_ptr<Frontend> m_frontend;   
    std::unique_ptr<Async> m_async;
    std::unique_ptr<EventFlow> m_event_flow;
    std::shared_ptr<Executor> m_control;
    
    mutable std::mutex m_mutex;
};

}} //namespace TR { namespace Core {