#pragma once
#include "RegistryFwd.h"
#include "RegistryTrivialTypes.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4521)
#pragma warning(disable:4522)

class SourceRef
{
public:
    friend class Sources;
    friend class Collector;
    template<class T>
    friend struct Unpacking;

    SourceKey   get_key() const;
    Source      operator*() const;
    bool        is_relevant() const;
    bool        is_same(const SourceRef& source_ref) const;
    bool        is_less(const SourceRef& source_ref) const;

private:
    SourceRef(const Sources* sources, SourceKey source_key);
    const Sources* m_sources;
    SourceKey m_source_key;
};

bool operator == (const SourceRef& left, const SourceRef& right);
bool operator != (const SourceRef& left, const SourceRef& right);
bool operator < (const SourceRef& left, const SourceRef& right);

class RoleRef
{
public:
    friend class Roles;
    friend class Sources;
    friend class FilterReports;
    template<class T>
    friend struct Unpacking;

    RoleKey get_key() const;
    Role    operator*() const;
    bool    is_relevant() const;
    bool    is_same(const RoleRef& role_ref) const;
    bool    is_less(const RoleRef& role_ref) const;
    bool    refer_default_role() const;

private:
    RoleRef(const Roles* roles, RoleKey role_key);
    const Roles* m_roles;
    RoleKey m_role_key;
};

bool operator == (const RoleRef& left, const RoleRef& right);
bool operator != (const RoleRef& left, const RoleRef& right);
bool operator < (const RoleRef& left, const RoleRef& right);

class CurrentReportRef
{
public:
    friend class Collector;

    SourceRef       get_source_ref() const;
    ReportTypeRef   get_report_type_ref() const;
    CurrentReport   operator*() const;
    bool            is_relevant() const;

private:
    CurrentReportRef(const Collector* collector, SourceRef source_ref, ReportTypeRef report_type_ref);
    const Collector* m_collector;
    SourceRef m_source_ref;
    ReportTypeRef m_report_type_ref;
};

using SubjectRef = boost::variant<SourceRef, RoleRef>;
SubjectRef                      get_subject_ref(Registry& registry, SubjectKey subject_key);
SubjectKey                      get_subject_key(SubjectRef subject_ref);
std::vector<SubjectRef>         get_subject_refs(Registry& registry);
std::vector<SubjectRef>         get_derived_refs(const Sources& sources, const Roles& roles, SubjectRef subject_ref, bool recursive = false);
std::vector<SubjectRef>         get_derived_refs(Registry& registry, SubjectRef subject_ref, bool recursive = false);
boost::optional<SubjectRef>     get_base_ref(SubjectRef subject_ref);
bool                            refer_default_role(SubjectRef subject_ref);

#pragma warning(pop)
}} //namespace TR { namespace Core {