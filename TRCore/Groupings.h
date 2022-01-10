#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {
class SourceRef;

struct GroupingDirection
{
    GroupingDirection(ReportTypeRef input_ref, ReportTypeRef output_ref);

    ReportTypeRef m_input_ref;
    ReportTypeRef m_output_ref;
};

bool operator < (const GroupingDirection& left, const GroupingDirection& right);
bool operator == (const GroupingDirection& left, const GroupingDirection& right);

class Grouping
{
public:
    using Functor = std::function<std::unique_ptr<Content>(const std::vector<std::shared_ptr<Content>>&)>;
    
    Grouping(UUID uuid) :
        m_uuid(std::move(uuid))
    {
    }
    virtual ~Grouping() = default;

    UUID                        get_uuid() const { return m_uuid; }

    virtual GroupingDirection   get_direction() const = 0;
    virtual Functor             prepare_functor(SourceRef source_ref, const std::vector<SourceRef>& source_refs) const = 0;

private:
    UUID m_uuid;
};

class Groupings
{
public:
    static const Priority GROUPINGS_PRIORITY = 600;

    Groupings();
    ~Groupings();
    void                                        set_report_types(ReportTypes* report_types);

    void                                        add_grouping(std::shared_ptr<Grouping> grouping, Transaction& t);
    void                                        remove_grouping(const GroupingDirection& direction, Transaction& t);
    std::vector<std::shared_ptr<Grouping>>      get_groupings() const;
    std::shared_ptr<Grouping>                   get_grouping(const GroupingDirection& direction) const;
    std::shared_ptr<Grouping>                   find_grouping(const GroupingDirection& direction) const;
    std::vector<std::shared_ptr<Grouping>>      find_by_input(ReportTypeRef input_ref) const;
    std::vector<std::shared_ptr<Grouping>>      find_by_output(ReportTypeRef output_ref) const;

    template<class T>
    void connect_add_grouping(T slot, Priority priority) { m_add_grouping_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_grouping(T slot, Priority priority) { m_remove_grouping_sig.connect(slot, priority); }

private:
    class Storage;

    OrderedSignal<void(GroupingDirection direction, Transaction& t)> m_add_grouping_sig;
    OrderedSignal<void(GroupingDirection direction, Transaction& t)> m_remove_grouping_sig;

    ReportTypes* m_report_types;
    std::unique_ptr<Storage> m_storage;
};

}} //namespace TR { namespace Core {
