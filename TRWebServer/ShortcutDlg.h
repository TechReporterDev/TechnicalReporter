#pragma once
#include "AppClient.h"
#include "TaskDlg.h"

class ShortcutDlg: public TaskDlg
{
public:
    ShortcutDlg(const TR::SourceInfo& sourceInfo, const TR::ActionShortcutInfo& shortcutInfo);

    void apply() override;

private:
    Wt::WCheckBox* m_showOutputCheck;

    TR::Client::Job m_job;
    TR::SourceInfo m_sourceInfo;
    TR::ActionShortcutInfo m_shortcutInfo;
    std::shared_ptr<TR::Content> m_output;
};

class ShortcutBundleDlg: public TaskBundleDlg
{
public:
    ShortcutBundleDlg(std::vector<TR::SourceInfo> sourcesInfo, TR::ActionShortcutInfo shortcutInfo);
    void showSummary();

    // Dialog override
    void apply() override;

private:
    std::queue<TR::Client::Job> m_jobs;
    std::vector<TR::SourceInfo> m_sourcesInfo;
    TR::ActionShortcutInfo m_shortcutInfo;
    TR::Params m_params;
};
