#pragma once
#include "Tasks.h"

namespace TR { namespace Core {
class SummaryPdf;

class ExportSummary : public Task, public std::enable_shared_from_this<ExportSummary>
{
public:
    ExportSummary(CoreDomain& core_domain, Scheduler& scheduler, Key user_key, Executive& executive);
    Blob get_output();

protected:
    // Job override
    virtual void do_run() override;

protected:
    CoreDomain& m_core_domain;
    Scheduler& m_scheduler;
    Sources& m_sources;
    ReportTypes& m_report_types;
    SourceResources& m_source_resources;

    Key m_user_key;
    std::shared_ptr<SummaryPdf> m_summaryPdf;
    std::unique_ptr<std::ostream> m_ostream;
    Blob m_output;
};

}} //namespace TR { namespace Core {