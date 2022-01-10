#include "stdafx.h"
#include "ShortcutDlg.h"
#include "TaskDlg.h"
#include "TaskOutputPage.h"
#include "Application.h"

ShortcutDlg::ShortcutDlg(const TR::SourceInfo& sourceInfo, const TR::ActionShortcutInfo& shortcutInfo):
    TaskDlg(shortcutInfo.m_name, L"Action running..."),
    m_showOutputCheck(nullptr),
    m_sourceInfo(sourceInfo),   
    m_shortcutInfo(shortcutInfo)
{
    auto& client = Application::instance()->getClient();
    m_job = client.launchShortcut(sourceInfo.m_key, shortcutInfo.m_uuid,
        [this](std::shared_ptr<TR::Content> output){
            m_output = output;
            if (output)
            {
                m_showOutputCheck = expand(std::make_unique<Wt::WCheckBox>("show recieved content"));
                m_showOutputCheck->setChecked(true);
            }
            setCompleted("Action completed");
        },
        [this](const std::string& err){
            setFailed("Action failed!", err);
        }
    );
}

void ShortcutDlg::apply()
{
    if (m_showOutputCheck && m_showOutputCheck->isChecked())
    {
        Application::showPage(std::make_unique<TaskOutputPage>(L"Shortcut launch output", m_sourceInfo, m_output));
    }
}

ShortcutBundleDlg::ShortcutBundleDlg(std::vector<TR::SourceInfo> sourcesInfo, TR::ActionShortcutInfo shortcutInfo):
    TaskBundleDlg(shortcutInfo.m_name, L"Running..."),
    m_sourcesInfo(std::move(sourcesInfo)),
    m_shortcutInfo(std::move(shortcutInfo))
{
    _ASSERT(!m_sourcesInfo.empty());

    auto& client = Application::instance()->getClient();
    for (auto& sourceInfo : m_sourcesInfo)
    {
        m_jobs.push(client.launchShortcut(sourceInfo.m_key, m_shortcutInfo.m_uuid,
            [this](std::shared_ptr<TR::Content>){
                auto& job = m_jobs.front();
                setCompleted(job.get_id(), "Completed");
                m_jobs.pop();

                if (m_jobs.empty())
                {
                    showSummary();
                }
            },
            [this](const std::string& err){
                auto& job = m_jobs.front();
                setFailed(job.get_id(), "Failed", err);
                m_jobs.pop();

                if (m_jobs.empty())
                {
                    showSummary();
                }
            })
        );

        addTask(m_jobs.back().get_id(), sourceInfo.m_name, L"Running...");
    }
}

void ShortcutBundleDlg::showSummary()
{
    if (getFailedCount() > 0)
    {
        setFailed("Action failed!");
    }
    else
    {
        setCompleted("Action completed.");
    }
}

void ShortcutBundleDlg::apply()
{
}