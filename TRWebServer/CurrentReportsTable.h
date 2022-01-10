#pragma once
#include "StandardTable.h"
#include "TRWebServer.h"
#include "MainFrame.h"

using ResourceInfo = boost::variant<TR::CurrentReportInfo, TR::StreamInfo>;
using CurrentReportRow = StandardModelRow<ResourceInfo>;
using CurrentReportsTableColumn = StandardTableColumn<ResourceInfo>;

class CurrentReportNameColumn: public CurrentReportsTableColumn
{
public:
    CurrentReportNameColumn(std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& reportTypes, std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>& streamTypes);
    
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const CurrentReportRow& row) const override;

private:
    std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& m_reportTypes;
    std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>& m_streamTypes;
};

class CurrentReportModifiedDateColumn: public CurrentReportsTableColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const CurrentReportRow& row) const override;
};

class CurrentReportUpdatedDateColumn: public CurrentReportsTableColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const CurrentReportRow& row) const override;
};

class CurrentReportsTable: public StandardTable<ResourceInfo>
{
public:
    CurrentReportsTable();

private:
    std::map<TR::ReportTypeUUID, TR::ReportTypeInfo> m_reportTypes;
    std::map<TR::StreamTypeUUID, TR::StreamTypeInfo> m_streamTypes;
};