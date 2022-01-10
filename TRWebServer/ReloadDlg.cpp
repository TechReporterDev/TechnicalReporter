#include "stdafx.h"
#include "ReloadDlg.h"
#include "TaskOutputPage.h"
#include "Application.h"

ReloadDlg::ReloadDlg(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo):
    TaskDlg(reportTypeInfo.m_name, L"Loading..."),  
    m_sourceInfo(sourceInfo),   
    m_reportTypeInfo(reportTypeInfo)
{
    auto& client = Application::instance()->getClient();
    m_job = client.reloadReport(sourceInfo.m_key, reportTypeInfo.m_uuid,
        [this](std::shared_ptr<TR::Content> output){
            m_output = output;
            setCompleted("Loading is completed and processing started");

            if (output)
            {
                m_showOutputCheck = expand(std::make_unique<Wt::WCheckBox>("show loaded report"));              
                m_showOutputCheck->setMargin(0);
                m_showOutputCheck->setChecked(false);
            }
        },
        [this](const std::string& err){
            setFailed("Loading failed!", err);
        }
    );
}

void ReloadDlg::apply()
{
    if (m_output && m_showOutputCheck->isChecked())
    {
        Application::showPage(std::make_unique<TaskOutputPage>(L"Downloaded content", m_sourceInfo, m_output));
    }
}

ReloadAllDlg::ReloadAllDlg(const TR::SourceInfo& sourceInfo):
    TaskBundleDlg(L"Reload", L"Loading..."),
    m_sourceInfo(sourceInfo)
{
    auto& client = Application::instance()->getClient();
    auto sourceTypeInfo = client.getSourceTypeInfo(m_sourceInfo.m_source_type_uuid);
    std::map<TR::UUID, TR::ReportTypeInfo> reportTypes;
    for (auto& reportTypeInfo: client.getReportTypesInfo())
    {
        reportTypes[reportTypeInfo.m_uuid] = reportTypeInfo;
    }

    for (auto uuid : sourceTypeInfo.m_download_uuids)
    {
        m_jobs.push(client.reloadReport(sourceInfo.m_key, uuid,
            [this](std::shared_ptr<TR::Content> output){
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

        addTask(m_jobs.back().get_id(), reportTypes[uuid].m_name, L"Loading...");
    }       
}

void ReloadAllDlg::showSummary()
{
    if (getFailedCount() > 0)
    {
        setFailed("Loading failed!");
    }
    else
    {
        setCompleted("Loading is completed and processing started.");
    }
}

void ReloadAllDlg::apply()
{       
}

ReloadBundleDlg::ReloadBundleDlg(std::vector<TR::SourceInfo> sourcesInfo, TR::ReportTypeInfo reportTypeInfo):
    TaskBundleDlg(L"Reload", L"Loading..."),
    m_sourcesInfo(std::move(sourcesInfo)),
    m_reportTypeInfo(std::move(reportTypeInfo))
{
    _ASSERT(!m_sourcesInfo.empty());

    auto& client = Application::instance()->getClient();
    for (auto& sourceInfo : m_sourcesInfo)
    {
        m_jobs.push(client.rebuildReport(sourceInfo.m_key, m_reportTypeInfo.m_uuid,
            [this](){
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

        addTask(m_jobs.back().get_id(), sourceInfo.m_name, L"Loading...");
    }
}

void ReloadBundleDlg::showSummary()
{
    if (getFailedCount() > 0)
    {
        setFailed("Loading failed!");
    }
    else
    {
        setCompleted("Loading is completed and processing started.");
    }
}

void ReloadBundleDlg::apply()
{
}