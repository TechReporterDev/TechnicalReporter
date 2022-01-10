#pragma once
#include "AppDecl.h"
#include "EventDispatcher.h"
namespace TR { namespace Core {
class EventFlow;
struct CoreImpl;
class Async;

struct SourceSubject
{
    SourceKey m_source_key;
};

bool operator < (const SourceSubject& left, const SourceSubject& right);

struct ReportSubject
{
    ReportTypeUUID m_report_type_uuid;
    SourceKey m_source_key;
};

bool operator < (const ReportSubject& left, const ReportSubject& right);

struct LinkSubject
{
    SourceKey m_parent_key;
    SourceKey m_child_key;
};

bool operator < (const LinkSubject& left, const LinkSubject& right);

using CoreSubject = boost::variant<SourceSubject, ReportSubject, LinkSubject>;

class Event
{
public:
    enum Cathegory {ADD_UPDATE_EVENT, REMOVE_EVENT};
    Event(EventFlow& event_flow, Cathegory cathegory);
    virtual ~Event(){}

    Cathegory                           get_cathegory() const;
    virtual CoreSubject                 get_subject() const = 0;
    virtual std::vector<CoreSubject>    get_relations() const = 0;
    virtual std::unique_ptr<Event>      combine(const Event& event) const = 0;
    virtual void                        emit() const = 0;

protected:
    template <class Functor> 
    void dispatch(Functor& f) const
    {
        m_event_flow.dispatch_event(f);
    }

    EventFlow& m_event_flow;
    CoreImpl& m_core_impl;
    Security& m_security;
    Cathegory m_cathegory;
};

class SourceEvent: public Event
{
public:
    SourceEvent(EventFlow& event_flow, Cathegory cathegory, SourceKey source_key);
    virtual CoreSubject                 get_subject() const override;
    virtual std::vector<CoreSubject>    get_relations() const override;

protected:
    Sources& m_sources;
    SourceKey m_source_key;
};

class AddSourceEvent: public SourceEvent
{
public:
    AddSourceEvent(EventFlow& event_flow, SourceRef source_ref);
    
    virtual std::unique_ptr<Event>      combine(const Event& evt) const override;
    virtual void                        emit() const override;
};

class UpdateSourceEvent: public SourceEvent
{
public:
    UpdateSourceEvent(EventFlow& event_flow, SourceRef source_ref);

    virtual std::unique_ptr<Event>      combine(const Event& evt) const override;
    virtual void                        emit() const override;
};

class RemoveSourceEvent: public SourceEvent
{
public:
    RemoveSourceEvent(EventFlow& event_flow, SourceKey source_key, std::vector<SourceKey> removed_sources);

    virtual std::vector<CoreSubject>    get_relations() const override;
    virtual std::unique_ptr<Event>      combine(const Event& evt) const override;
    virtual void                        emit() const override;

private:
    std::vector<SourceKey> m_removed_sources;
};

class LinkEvent: public Event
{
public:
    LinkEvent(EventFlow& event_flow, Cathegory cathegory, Link link);

    virtual CoreSubject                 get_subject() const override;
    virtual std::vector<CoreSubject>    get_relations() const override;

protected:
    SourceKey m_parent_key;
    SourceKey m_child_key;
};

class AddLinkEvent: public LinkEvent
{
public:
    AddLinkEvent(EventFlow& event_flow, Link link);
    virtual std::unique_ptr<Event>      combine(const Event& evt) const override;
    virtual void                        emit() const override;
};

class RemoveLinkEvent: public LinkEvent
{
public:
    RemoveLinkEvent(EventFlow& event_flow, Link link);
    virtual std::unique_ptr<Event>      combine(const Event& evt) const override;
    virtual void                        emit() const override;
};

class UpdateReportEvent: public Event
{
public:
    UpdateReportEvent(EventFlow& event_flow, CurrentReportRef current_report_ref, bool content_changed);

    virtual CoreSubject                 get_subject() const override;
    virtual std::vector<CoreSubject>    get_relations() const override;
    virtual std::unique_ptr<Event>      combine(const Event& evt) const override;
    virtual void                        emit() const override;

private:
    SourceKey m_source_key;
    ReportTypeUUID m_report_type_uuid;
    bool m_content_changed;
};

class EventBatch
{
public:
    bool empty() const;
    void add_event(std::unique_ptr<Event> event_ptr);   
    void emit();    

private:
    std::map<CoreSubject, std::unique_ptr<Event>> m_events;
};

class EventFlow
{
public:
    friend class Event;
    typedef std::vector<std::unique_ptr<EventDispatcher>> EventDispatchers;
    static const Priority EVENT_FLOW_PRIORITY = 15000;

    EventFlow(CoreImpl& core_impl);

    void set_async(Async* async);
    void set_dispatchers(EventDispatchers* dispatchers);

private:
    template <class Functor> 
    void dispatch_event(Functor& f) const
    {
        for (auto& dispatcher : *m_dispatchers | boost::adaptors::indirected)
        {
            f(dispatcher);
        }
    }

    EventBatch&     get_event_batch(Transaction& t);
    void            emit_event_batch();
    void            reset_event_batch();
    
    CoreImpl& m_core_impl;
    Security& m_security;
    Async* m_async;
    EventDispatchers* m_dispatchers;
    std::unique_ptr<EventBatch> m_event_batch;
};

}}//namespace TR { namespace Core {