#include "stdafx.h"
#include "RegistryRefs.h"
#include "Registry.h"
namespace TR { namespace Core {

SourceKey SourceRef::get_key() const
{
    return m_source_key;
}

Source SourceRef::operator*() const
{
    return m_sources->get_source(m_source_key);
}

bool SourceRef::is_relevant() const
{
    return m_sources->has_source(m_source_key);
}

bool SourceRef::is_same(const SourceRef& source_ref) const
{
    return m_sources == source_ref.m_sources && m_source_key == source_ref.m_source_key;
}

bool SourceRef::is_less(const SourceRef& source_ref) const
{
    _ASSERT(m_sources == source_ref.m_sources);
    return m_source_key < source_ref.m_source_key;
}

SourceRef::SourceRef(const Sources* sources, SourceKey source_key):
    m_sources(sources),
    m_source_key(source_key)
{
    _ASSERT(m_sources);
    _ASSERT(m_source_key);
}

bool operator == (const SourceRef& left, const SourceRef& right)
{
    return left.is_same(right);
}

bool operator != (const SourceRef& left, const SourceRef& right)
{
    return  !(left == right);
}

bool operator < (const SourceRef& left, const SourceRef& right)
{
    return left.is_less(right);
}

RoleKey RoleRef::get_key() const
{
    return m_role_key;
}

Role RoleRef::operator*() const
{
    return m_roles->get_role(m_role_key);
}

bool RoleRef::is_relevant() const
{
    return m_roles->has_role(m_role_key);
}

bool RoleRef::is_same(const RoleRef& role_ref) const
{
    return m_roles == role_ref.m_roles && m_role_key == role_ref.m_role_key;
}

bool RoleRef::is_less(const RoleRef& role_ref) const
{
    _ASSERT(m_roles == role_ref.m_roles);
    return m_role_key < role_ref.m_role_key;
}

bool RoleRef::refer_default_role() const
{
    return m_role_key == 1;
}

RoleRef::RoleRef(const Roles* roles, RoleKey role_key):
    m_roles(roles),
    m_role_key(role_key)
{
}

bool operator == (const RoleRef& left, const RoleRef& right)
{
    return left.is_same(right);
}

bool operator != (const RoleRef& left, const RoleRef& right)
{
    return !(left == right);
}

bool operator < (const RoleRef& left, const RoleRef& right)
{
    return left.is_less(right);
}

SourceRef CurrentReportRef::get_source_ref() const
{
    return m_source_ref;
}

ReportTypeRef CurrentReportRef::get_report_type_ref() const
{
    return m_report_type_ref;
}

CurrentReport CurrentReportRef::operator*() const
{
    return m_collector->get_current_report(m_source_ref, m_report_type_ref);
}

bool CurrentReportRef::is_relevant() const
{
    return m_collector->find_current_report(m_source_ref, m_report_type_ref).is_initialized();
}

CurrentReportRef::CurrentReportRef(const Collector* collector, SourceRef source_ref, ReportTypeRef report_type_ref):
    m_collector(collector),
    m_source_ref(source_ref),
    m_report_type_ref(report_type_ref)
{
}

SubjectRef get_subject_ref(Registry& registry, SubjectKey subject_key)
{
    if (auto role_key = boost::get<RoleKey>(&subject_key))
    {
        return registry.m_roles.get_role(*role_key);
    }

    auto source_key = boost::get<SourceKey>(subject_key);
    return registry.m_sources.get_source(source_key);
}

SubjectKey get_subject_key(SubjectRef subject_ref)
{
    if (auto role_ref = boost::get<RoleRef>(&subject_ref))
    {
        return role_ref->get_key();
    }
    return boost::get<SourceRef>(subject_ref).get_key();
}

std::vector<SubjectRef> get_subject_refs(Registry& registry)
{
    std::vector<SubjectRef> subject_refs;
    for (auto& child_role : registry.m_roles.get_roles())
    {
        subject_refs.push_back(child_role);
    }

    for (auto& source : registry.m_sources.get_sources())
    {
        subject_refs.push_back(source);
    }
    return subject_refs;
}

std::vector<SubjectRef> get_derived_refs(const Sources& sources, const Roles& roles, SubjectRef subject_ref, bool recursive)
{
    std::vector<SubjectRef> derived_refs;
    if (auto role_ref = boost::get<RoleRef>(&subject_ref))
    {
        for (auto& derived_role : roles.get_roles(*role_ref))
        {
            derived_refs.push_back(derived_role);
            if (recursive)
            {
                boost::copy(get_derived_refs(sources, roles, derived_role, true), std::back_inserter(derived_refs));
            }
        }

        for (auto& source : sources.get_sources(*role_ref))
        {
            derived_refs.push_back(source);
        }
    }
    return derived_refs;
}

std::vector<SubjectRef> get_derived_refs(Registry& registry, SubjectRef subject_ref, bool recursive)
{
    return get_derived_refs(registry.m_sources, registry.m_roles, subject_ref, recursive);
}

boost::optional<SubjectRef> get_base_ref(SubjectRef subject_ref)
{
    if (auto role_ref = boost::get<RoleRef>(&subject_ref))
    {
        auto role = **role_ref;
        if (!role.has_parent())
        {
            return boost::none;
        }
        return role.get_parent();
    }
    else if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        auto source = **source_ref;
        return source.get_role();
    }
    else
    {
        _ASSERT(false);
    }
    return boost::none;
}

bool refer_default_role(SubjectRef subject_ref)
{
    if (auto role_ref = boost::get<RoleRef>(&subject_ref))
    {
        return role_ref->refer_default_role();
    }
    return false;
}

}} //namespace TR { namespace Core {