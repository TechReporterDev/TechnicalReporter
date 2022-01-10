#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {
class SourceRef;

struct TrackingDirection
{
    TrackingDirection(ReportTypeRef input_ref, StreamTypeRef output_ref);

    ReportTypeRef m_input_ref;
    StreamTypeRef m_output_ref;
};

bool operator < (const TrackingDirection& left, const TrackingDirection& right);
bool operator == (const TrackingDirection& left, const TrackingDirection& right);

class Tracking
{
public:
    using Functor = std::function<std::vector<StreamMessage>(std::shared_ptr<Content>)>;
    
    virtual ~Tracking() = default;  
    virtual TrackingDirection   get_direction() const = 0;
    virtual Functor             prepare_functor(SourceRef source_ref) const = 0;
};

class Trackings
{
public:
    static const Priority TRACKINGS_PRIORITY = 650;
    
    Trackings();
    ~Trackings();
    void                                        set_report_types(ReportTypes* report_types);
    void                                        set_stream_types(StreamTypes* stream_types);

    void                                        add_tracking(std::shared_ptr<Tracking> tracking, Transaction& t);
    void                                        remove_tracking(const TrackingDirection& direction, Transaction& t);
    std::shared_ptr<Tracking>                   get_tracking(const TrackingDirection& direction) const;
    std::shared_ptr<Tracking>                   find_tracking(const TrackingDirection& direction) const;
    std::vector<std::shared_ptr<Tracking>>      find_by_input(ReportTypeRef input_ref) const;
    std::vector<std::shared_ptr<Tracking>>      find_by_output(StreamTypeRef output_ref) const;

    template<class T>
    void connect_add_tracking(T slot, Priority priority) { m_add_tracking_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_tracking(T slot, Priority priority) { m_remove_tracking_sig.connect(slot, priority); }

private:
    class Storage;  

    OrderedSignal<void(TrackingDirection direction, Transaction& t)> m_add_tracking_sig;
    OrderedSignal<void(TrackingDirection direction, Transaction& t)> m_remove_tracking_sig;

    ReportTypes* m_report_types;
    StreamTypes* m_stream_types;
    std::unique_ptr<Storage> m_storage;
};

}} //namespace TR { namespace Core {
