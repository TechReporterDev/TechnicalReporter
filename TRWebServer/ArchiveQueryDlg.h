#pragma once
#include "TRWebServer.h"
#include "PropertyDialog.h"

class ArchiveQueryDlg: public PropertyDialog
{
public:
    ArchiveQueryDlg(const TR::SourceInfo& sourceInfo);
    virtual void apply() override;

private:
    TR::SourceInfo m_sourceInfo;
    std::vector<TR::ReportTypeInfo> m_reportTypes;
    int m_reportTypePosition;
    std::tm m_date;
    std::tm m_time;
};
