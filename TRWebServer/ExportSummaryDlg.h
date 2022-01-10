#pragma once
#include "AppClient.h"
#include "TaskDlg.h"

class ExportSummaryDlg : public TaskDlg
{
public:
    ExportSummaryDlg();

    // Dialog override
    void apply() override;

private:
    TR::Client::Job m_job;
    std::shared_ptr<TR::Content> m_output;
};
