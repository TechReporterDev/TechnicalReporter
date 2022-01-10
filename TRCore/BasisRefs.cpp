#include "stdafx.h"
#include "BasisRefs.h"
#include "Basis.h"

namespace TR { namespace Core {

ReportTypeUUID ReportTypeRef::get_uuid() const
{
    return m_report_type_uuid;
}

const ReportType& ReportTypeRef::operator*() const
{
    return m_report_types->get_report_type(m_report_type_uuid);
}

bool ReportTypeRef::is_relevant() const
{
    return m_report_types->has_report_type(m_report_type_uuid);
}

ReportTypeRef::ReportTypeRef(const ReportTypes* report_types, ReportTypeUUID report_type_uuid):
    m_report_types(report_types),
    m_report_type_uuid(report_type_uuid)
{
    _ASSERT(m_report_types);
    _ASSERT(m_report_type_uuid != stl_tools::null_uuid());
}

bool operator < (const ReportTypeRef& left, const ReportTypeRef& right)
{
    return left.get_uuid() < right.get_uuid();
}

bool operator == (const ReportTypeRef& left, const ReportTypeRef& right)
{
    return left.get_uuid() == right.get_uuid();
}

bool operator != (const ReportTypeRef& left, const ReportTypeRef& right)
{
    return ! operator == (left, right);
}

std::vector<ReportTypeUUID> get_uuids(const std::vector<ReportTypeRef>& report_type_refs)
{
    std::vector<ReportTypeUUID> report_type_uuids;
    for (auto& report_type_ref : report_type_refs)
    {
        report_type_uuids.push_back(report_type_ref.get_uuid());
    }
    return report_type_uuids;
}

StreamTypeUUID StreamTypeRef::get_uuid() const
{
    return m_stream_type_uuid;
}

const StreamType& StreamTypeRef::operator*() const
{
    return m_stream_types->get_stream_type(m_stream_type_uuid);
}

bool StreamTypeRef::is_relevant() const
{
    return m_stream_types->has_stream_type(m_stream_type_uuid);
}

StreamTypeRef::StreamTypeRef(const StreamTypes* stream_types, StreamTypeUUID stream_type_uuid):
    m_stream_types(stream_types),
    m_stream_type_uuid(stream_type_uuid)
{
}

bool operator < (const StreamTypeRef& left, const StreamTypeRef& right)
{
    return left.get_uuid() < right.get_uuid();
}

bool operator == (const StreamTypeRef& left, const StreamTypeRef& right)
{
    return left.get_uuid() == right.get_uuid();
}

bool operator != (const StreamTypeRef& left, const StreamTypeRef& right)
{
    return !operator == (left, right);
}

UUID SourceTypeRef::get_uuid() const
{
    return m_source_type_uuid;
}

const SourceType& SourceTypeRef::operator*() const
{
    return m_source_types->get_source_type(m_source_type_uuid);
}

bool SourceTypeRef::is_relevant() const
{
    return m_source_types->has_source_type(m_source_type_uuid);
}

SourceTypeRef::SourceTypeRef(const SourceTypes* source_types, UUID source_type_uuid):
    m_source_types(source_types),
    m_source_type_uuid(source_type_uuid)
{
    _ASSERT(m_source_types);
    _ASSERT(! m_source_type_uuid.is_nil());
}

bool operator < (const SourceTypeRef& left, const SourceTypeRef& right)
{
    return left.get_uuid() < right.get_uuid();
}

bool operator == (const SourceTypeRef& left, const SourceTypeRef& right)
{
    return left.get_uuid() == right.get_uuid();
}

bool operator != (const SourceTypeRef& left, const SourceTypeRef& right)
{
    return left.get_uuid() != right.get_uuid();
}

ActionUUID ActionRef::get_uuid() const
{
    return m_action_uuid;
}

const Action& ActionRef::operator*() const
{
    return m_actions->get_action(m_action_uuid);
}

bool ActionRef::is_relevant() const
{
    return m_actions->has_action(m_action_uuid);
}

ActionRef::ActionRef(const Actions* actions, ActionUUID action_uuid):
    m_actions(actions),
    m_action_uuid(action_uuid)
{
}

bool operator < (const ActionRef& left, const ActionRef& right)
{
    return left.get_uuid() < right.get_uuid();
}

bool operator == (const ActionRef& left, const ActionRef& right)
{
    return left.get_uuid() == right.get_uuid();
}

UUID ActionShortcutRef::get_uuid() const
{
    return m_shortcut_uuid;
}

const ActionShortcut& ActionShortcutRef::operator*() const
{
    return m_shortcuts->get_shortcut(m_shortcut_uuid);
}

bool ActionShortcutRef::is_relevant() const
{
    return m_shortcuts->has_shortcut(m_shortcut_uuid);
}

bool ActionShortcutRef::is_less(const ActionShortcutRef& shortcut_ref) const
{
    _ASSERT(m_shortcuts == shortcut_ref.m_shortcuts);
    return m_shortcut_uuid < shortcut_ref.m_shortcut_uuid;
}


ActionShortcutRef::ActionShortcutRef(const ActionShortcuts* shortcuts, UUID shortcut_uuid):
    m_shortcuts(shortcuts),
    m_shortcut_uuid(shortcut_uuid)
{
}

bool operator < (const ActionShortcutRef& left, const ActionShortcutRef& right)
{
    return left.is_less(right);
}

ResourceUUID get_uuid(const ResourceRef& resource_ref)
{
    if (auto report_type_ref = boost::get<ReportTypeRef>(&resource_ref))
    {
        return report_type_ref->get_uuid();
    }

    auto stream_type_ref = boost::get<StreamTypeRef>(resource_ref);
    return stream_type_ref.get_uuid();
}

ResourceRef get_ref(const Basis& basis, ResourceUUID resource_uuid)
{
    if (resource_uuid.type() == typeid(ReportTypeUUID))
    {
        return basis.m_report_types.get_report_type(boost::get<ReportTypeUUID>(resource_uuid));
    }
    return basis.m_stream_types.get_stream_type(boost::get<StreamTypeUUID>(resource_uuid));
}

}} //namespace TR { namespace Core {