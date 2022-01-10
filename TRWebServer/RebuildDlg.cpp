#include "stdafx.h"
#include "RebuildDlg.h"
#include "TaskOutputPage.h"
#include "Application.h"

RebuildDlg::RebuildDlg(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo):
    TaskDlg(reportTypeInfo.m_name, L"(Re)loading of all dependencies..."),
    m_sourceInfo(sourceInfo),
    m_reportTypeInfo(reportTypeInfo)
{
    auto& client = Application::instance()->getClient();
    m_job = client.rebuildReport(sourceInfo.m_key, reportTypeInfo.m_uuid, 
        [this](){
            setCompleted("All dependencies successfully (re)loaded from device and processing started");        
        },
        [this](const std::string& err){
            setFailed("Loading failed!", err);
        }
    );
}