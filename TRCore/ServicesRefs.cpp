#include "stdafx.h"
#include "ServicesRefs.h"
#include "CustomQueries.h"
#include "GroupingReports.h"
#include "Compliance.h"
#include "Validation.h"
#include "Archive.h"
#include "CustomActions.h"
#include "RegistryRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {

Key CustomQueryRef::get_key() const
{
    return m_custom_query_key;
}

CustomQuery CustomQueryRef::operator*() const
{
    return m_custom_queries->get_custom_query(m_custom_query_key);
}

CustomQueryRef::CustomQueryRef(const CustomQueries* custom_queries, Key custom_query_key):
    m_custom_queries(custom_queries),
    m_custom_query_key(custom_query_key)
{
}

Key GroupingReportRef::get_key() const
{
    return m_grouping_report_key;
}

GroupingReport GroupingReportRef::operator*() const
{
    return m_grouping_reports->get_grouping_report(m_grouping_report_key);
}

GroupingReportRef::GroupingReportRef(const GroupingReports* grouping_reports, Key grouping_report_key):
    m_grouping_reports(grouping_reports),
    m_grouping_report_key(grouping_report_key)
{
}

ComplianceKey ComplianceRef::get_key() const
{
    return m_key;
}

Compliance ComplianceRef::operator*() const
{
    return m_compliances->get_compliance(m_key);
}

ComplianceRef::ComplianceRef(const Compliances* compliances, ComplianceKey key):
    m_compliances(compliances),
    m_key(key)
{
}

bool operator < (const ComplianceRef& left, const ComplianceRef& right)
{
    return left.get_key() < right.get_key();
}

ValidationKey ValidationRef::get_key() const
{
    return m_key;
}

Validation ValidationRef::operator*() const
{
    return m_validations->get_validation(m_key);
}

ValidationRef::ValidationRef(const Validations* validations, ValidationKey key):
    m_validations(validations),
    m_key(key)
{
}

bool operator < (const ValidationRef& left, const ValidationRef& right)
{
    return left.get_key() < right.get_key();
}

ArchivedReportKey ArchivedReportRef::get_key() const
{
    return m_archived_report_key;
}

ArchivedReport ArchivedReportRef::operator*() const
{
    return m_archive->get_archived_report(m_archived_report_key);
}

ArchivedReportRef::ArchivedReportRef(const Archive* archive, ArchivedReportKey archived_report_key):
    m_archive(archive),
    m_archived_report_key(archived_report_key)
{
}

UUID CustomActionRef::get_uuid() const
{
    return m_uuid;
}

CustomAction CustomActionRef::operator*() const
{
    return m_custom_actions->get_custom_action(m_uuid);
}

CustomActionRef::CustomActionRef(const CustomActions* custom_actions, UUID uuid):
    m_custom_actions(custom_actions),
    m_uuid(uuid)
{
}

}} //namespace TR { namespace Core {