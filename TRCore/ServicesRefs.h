#pragma once
#include "ServicesFwd.h"
#include "ServicesTrivialTypes.h"
#include "stddecl.h"
namespace TR { namespace Core {

class CustomQueryRef
{
public:
    friend class CustomQueries;
    Key             get_key() const;
    CustomQuery     operator*() const;

private:
    CustomQueryRef(const CustomQueries* custom_queries, Key custom_query_key);
    const CustomQueries* m_custom_queries;
    Key m_custom_query_key;
};

class GroupingReportRef
{
public:
    friend class GroupingReports;
    Key             get_key() const;
    GroupingReport  operator*() const;

private:
    GroupingReportRef(const GroupingReports* grouping_reports, Key grouping_report_key);
    const GroupingReports* m_grouping_reports;
    Key m_grouping_report_key;
};

class ComplianceRef
{
public:
    friend class Compliances;
    ComplianceKey   get_key() const;
    Compliance      operator*() const;

private:
    ComplianceRef(const Compliances* compliances, ComplianceKey key);
    const Compliances* m_compliances;
    ComplianceKey m_key;
};

bool operator < (const ComplianceRef& left, const ComplianceRef& right);

class ValidationRef
{
public:
    friend class Validations;
    ValidationKey   get_key() const;
    Validation      operator*() const;

private:
    ValidationRef(const Validations* validations, ValidationKey key);
    const Validations* m_validations;
    ValidationKey m_key;
};

bool operator < (const ValidationRef& left, const ValidationRef& right);

class ArchivedReportRef
{
public:
    friend class Archive;

    ArchivedReportKey   get_key() const;
    ArchivedReport      operator*() const;

private:
    ArchivedReportRef(const Archive* archive, ArchivedReportKey archived_report_key);
    const Archive* m_archive;
    ArchivedReportKey m_archived_report_key;
};

class CustomActionRef
{
public:
    friend class CustomActions;
    UUID            get_uuid() const;
    CustomAction    operator*() const;

private:
    CustomActionRef(const CustomActions* custom_actions, UUID uuid);
    const CustomActions* m_custom_actions;
    UUID m_uuid;
};

}} //namespace TR { namespace Core {