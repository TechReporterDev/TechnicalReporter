#pragma once
#include "AppClient.h"
#include "PropertyDialog.h"
#include "TaskDlg.h"

class ActionDlg: public TaskDlg
{
public:
    ActionDlg(const TR::SourceInfo& sourceInfo, const TR::ActionInfo& actionInfo, TR::Params params);

    // Dialog overide
    void apply() override;

private:
    Wt::WCheckBox* m_showOutputCheck;

    TR::Client::Job m_job;
    TR::SourceInfo m_sourceInfo;
    TR::ActionInfo m_actionInfo;
    TR::Params m_params;
    std::shared_ptr<TR::Content> m_output;
};

class ActionBundleDlg: public TaskBundleDlg
{
public:
    ActionBundleDlg(std::vector<TR::SourceInfo> sourcesInfo, TR::ActionInfo actionInfo, TR::Params params);
    void showSummary();

    // Dialog override
    void apply() override;

private:
    std::queue<TR::Client::Job> m_jobs;
    std::vector<TR::SourceInfo> m_sourcesInfo;
    TR::ActionInfo m_actionInfo;
    TR::Params m_params;
};

class LaunchDlg: public PropertyDialog
{
public:
    LaunchDlg(const TR::SourceInfo& sourceInfo, const TR::ActionInfo& actionInfo, TR::Params params);

    // Dialog override
    virtual void apply() override;

private:
    TR::SourceInfo m_sourceInfo;
    TR::ActionInfo m_actionInfo;
    TR::Params m_params;
};