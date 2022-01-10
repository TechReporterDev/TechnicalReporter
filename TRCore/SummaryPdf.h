#pragma once
#include "AppDecl.h"

namespace PoDoFo
{
class PdfStreamedDocument;
class PdfOutputDevice;
class PdfCursorPainter;
}

namespace TR { namespace Core {

struct SourceCounters
{
    int m_count = 0;
    int m_ok = 0;
    int m_errors = 0;
    int m_warnings = 0;
};

struct ReportCounters
{
    int m_count = 0;
    int m_intrgrity_checking_errors = 0;
    int m_compliance_errors = 0;
    int m_validation_errors = 0;
};

struct Alert
{
    enum class Reason {
        INTEGRITY_CHECKING_FAILED,
        COMPLIANCE_FAILED,
        VALIDATION_FAILED,
        LOADING_FAILED
    };  

    std::wstring m_source_name;
    std::wstring m_report_name;
    Reason m_reason;
};

class SummaryPdf
{
public:
    SummaryPdf(const std::wstring& path);
    SummaryPdf(const std::ostream* ostream);
    ~SummaryPdf();

    void print_overview_title();
    void print_source_counters(const SourceCounters& source_counters);
    void print_report_counters(const ReportCounters& report_counters);
    void print_alerts(const std::vector<Alert>& alerts);
    void print_details_title();
    void print_source_title(const std::wstring& source_name, const std::wstring& source_type);
    void print_source_config(std::shared_ptr<const XmlPropertiesDoc> config);
    void print_source_issues_title();
    void print_source_has_no_issues();
    void print_integrity_checking_alert(const std::wstring& report_name, std::shared_ptr<const Diff> diff);
    void close();

protected:
    std::unique_ptr<PoDoFo::PdfOutputDevice> m_output;
    std::unique_ptr<PoDoFo::PdfStreamedDocument> m_pdf;
    std::unique_ptr<PoDoFo::PdfCursorPainter> m_painter;
};

}} //namespace TR { namespace Core {
