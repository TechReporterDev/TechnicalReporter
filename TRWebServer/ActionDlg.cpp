#include "stdafx.h"
#include "ActionDlg.h"
#include "CommonPropertyDoc.h"
#include "CommonPropertyPage.h"
#include "ActionOutputPage.h"
#include "Application.h"

ActionDlg::ActionDlg(const TR::SourceInfo& sourceInfo, const TR::ActionInfo& actionInfo, TR::Params params):
    TaskDlg(actionInfo.m_name, L"Running..."),
    m_showOutputCheck(nullptr),
    m_sourceInfo(sourceInfo),
    m_actionInfo(actionInfo),
    m_params(std::move(params))
{       
    auto& client = Application::instance()->getClient();
    m_job = client.launchAction(sourceInfo.m_key, actionInfo.m_uuid, m_params.clone(),
        [this](std::shared_ptr<TR::Content> output){
            m_output = output;
            if (output)
            {
                m_showOutputCheck = expand(std::make_unique<Wt::WCheckBox>("show recieved content"));
                m_showOutputCheck->setMargin(0);
                m_showOutputCheck->setChecked(true);
            }
            setCompleted("Action completed.");          
        },
        [this](const std::string& err){
            setFailed("Action failed!", err);
        }
    );
}

void ActionDlg::apply()
{
    if (m_showOutputCheck && m_showOutputCheck->isChecked())
    {
        Application::showPage(std::make_unique<ActionOutputPage>(m_sourceInfo, m_actionInfo, std::move(m_params), m_output));
    }
}

ActionBundleDlg::ActionBundleDlg(std::vector<TR::SourceInfo> sourcesInfo, TR::ActionInfo actionInfo, TR::Params params):
    TaskBundleDlg(actionInfo.m_name, L"Running..."),
    m_sourcesInfo(std::move(sourcesInfo)),
    m_actionInfo(std::move(actionInfo)),
    m_params(std::move(params))
{
    _ASSERT(!m_sourcesInfo.empty());

    auto& client = Application::instance()->getClient();
    for (auto& sourceInfo : m_sourcesInfo)
    {
        m_jobs.push(client.launchAction(sourceInfo.m_key, actionInfo.m_uuid, m_params.clone(),
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

void ActionBundleDlg::showSummary()
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

void ActionBundleDlg::apply()
{
}

LaunchDlg::LaunchDlg(const TR::SourceInfo& sourceInfo, const TR::ActionInfo& actionInfo, TR::Params params):
    PropertyDialog(actionInfo.m_name),
    m_sourceInfo(sourceInfo),
    m_actionInfo(actionInfo),
    m_params(std::move(params))
{
    init(std::make_unique<CommonPropertyPage>(createProperty(*m_params.m_params_doc)));
}

void LaunchDlg::apply()
{
    auto actionDlg = new ActionDlg(m_sourceInfo, m_actionInfo, std::move(m_params));
    actionDlg->show();
}
