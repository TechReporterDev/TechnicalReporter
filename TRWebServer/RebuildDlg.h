#pragma once
#include "AppClient.h"
#include "TaskDlg.h"

class RebuildDlg: public TaskDlg
{
public:
    RebuildDlg(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo);

private:
    TR::Client::Job m_job;
    TR::SourceInfo m_sourceInfo;
    TR::ReportTypeInfo m_reportTypeInfo;
};