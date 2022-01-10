#pragma once
#include "BasisFwd.h"
#include "BasisTrivialTypes.h"
#include "stddecl.h"
namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4521)
#pragma warning(disable:4522)

class ReportTypeRef
{
public:
    friend class ReportTypes;
    friend class Collector;
    template<class T> 
    friend struct Unpacking;

    ReportTypeUUID      get_uuid() const;
    const ReportType&   operator*() const;
    bool                is_relevant() const;

private:
    ReportTypeRef(const ReportTypes* report_types, ReportTypeUUID report_type_uuid);
    const ReportTypes* m_report_types;
    ReportTypeUUID m_report_type_uuid;
};

bool operator < (const ReportTypeRef& left, const ReportTypeRef& right);
bool operator == (const ReportTypeRef& left, const ReportTypeRef& right);
bool operator != (const ReportTypeRef& left, const ReportTypeRef& right);
std::vector<ReportTypeUUID> get_uuids(const std::vector<ReportTypeRef>& report_type_refs);

class StreamTypeRef
{
public:
    friend class StreamTypes;
    template<class T>
    friend struct Unpacking;

    StreamTypeUUID      get_uuid() const;
    const StreamType&   operator*() const;
    bool                is_relevant() const;

private:
    StreamTypeRef(const StreamTypes* stream_types, StreamTypeUUID stream_type_uuid);
    const StreamTypes* m_stream_types;
    StreamTypeUUID m_stream_type_uuid;
};

bool operator < (const StreamTypeRef& left, const StreamTypeRef& right);
bool operator == (const StreamTypeRef& left, const StreamTypeRef& right);
bool operator != (const StreamTypeRef& left, const StreamTypeRef& right);

class SourceTypeRef
{
public:
    friend class SourceTypes;
    template<class T>
    friend struct Unpacking;    

    UUID                get_uuid() const;
    const SourceType&   operator*() const;
    bool                is_relevant() const;

private:
    SourceTypeRef(const SourceTypes* source_types, UUID source_type_uuid);
    const SourceTypes* m_source_types;
    UUID m_source_type_uuid;
};

bool operator < (const SourceTypeRef& left, const SourceTypeRef& right);
bool operator == (const SourceTypeRef& left, const SourceTypeRef& right);
bool operator != (const SourceTypeRef& left, const SourceTypeRef& right);

class ActionRef
{
public:
    friend class Actions;
    template<class T>
    friend struct Unpacking;

    ActionUUID      get_uuid() const;
    const Action&   operator*() const;
    bool            is_relevant() const;

private:
    ActionRef(const Actions* actions, ActionUUID action_uuid);
    const Actions* m_actions;
    ActionUUID m_action_uuid;
};

bool operator < (const ActionRef& left, const ActionRef& right);
bool operator == (const ActionRef& left, const ActionRef& right);

class ActionShortcutRef
{
public:
    friend class ActionShortcuts;

    UUID                        get_uuid() const;
    const ActionShortcut&       operator*() const;
    bool                        is_relevant() const;
    bool                        is_less(const ActionShortcutRef& shortcut_ref) const;

private:
    ActionShortcutRef(const ActionShortcuts* shortcuts, UUID shortcut_uuid);
    const ActionShortcuts* m_shortcuts;
    UUID m_shortcut_uuid;
};

bool operator < (const ActionShortcutRef& left, const ActionShortcutRef& right);

using ResourceRef = boost::variant<ReportTypeRef, StreamTypeRef>;
ResourceUUID get_uuid(const ResourceRef& resource_ref);
ResourceRef get_ref(const Basis& basis, ResourceUUID resource_uuid);

#pragma warning(pop)
}} //namespace TR { namespace Core {