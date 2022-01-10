#pragma once
#include "AppClient.h"
#include "TaskDlg.h"

class ReloadDlg: public TaskDlg
{
public:
    ReloadDlg(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo);

    // Dialog override
    void apply() override;

private:
    Wt::WCheckBox* m_showOutputCheck;

    TR::Client::Job m_job;
    TR::SourceInfo m_sourceInfo;
    TR::ReportTypeInfo m_reportTypeInfo;
    std::shared_ptr<TR::Content> m_output;
};

class ReloadAllDlg: public TaskBundleDlg
{
public:
    ReloadAllDlg(const TR::SourceInfo& sourceInfo);
    void showSummary();

    // Dialog override
    void apply() override;

private:
    std::queue<TR::Client::Job> m_jobs;
    TR::SourceInfo m_sourceInfo;
};

class ReloadBundleDlg: public TaskBundleDlg
{
public:
    ReloadBundleDlg(std::vector<TR::SourceInfo> sourcesInfo, TR::ReportTypeInfo reportTypeInfo);
    void showSummary();

    // Dialog override
    void apply() override;

private:
    std::queue<TR::Client::Job> m_jobs; 
    std::vector<TR::SourceInfo> m_sourcesInfo;
    TR::ReportTypeInfo m_reportTypeInfo;
};