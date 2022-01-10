#pragma once
#include "StandardTable.h"
#include "TRWebServer.h"
#include "MainFrame.h"

using ChangeInfo = boost::variant<TR::CurrentReportInfo, TR::StreamInfo>;
using ChangesTableRow = StandardModelRow<ChangeInfo>;
using ChangesTableColumn = StandardTableColumn<ChangeInfo>;

class ChangeTimeColumn: public ChangesTableColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const ChangesTableRow& row) const override;
};

class ChangeSourceColumn: public ChangesTableColumn
{
public:
    ChangeSourceColumn(const std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo);
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const ChangesTableRow& row) const override;

private:
    const std::map<TR::SourceKey, TR::SourceInfo>& m_sourcesInfo;
};

class ChangeResourceColumn: public ChangesTableColumn
{
public:
    ChangeResourceColumn(const std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& reportTypesInfo, const std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>& streamTypesInfo);
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const ChangesTableRow& row) const override;

private:
    const std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& m_reportTypesInfo;
    const std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>& m_streamTypesInfo;  
};

class ChangesTable: public StandardTable<ChangeInfo>
{
public:
    ChangesTable();
    TR::SourceKey getSourceKey(size_t row) const;
};