#include "stdafx.h"
#include "SummaryPdf.h"
#include <podofo/podofo.h>
#include "PdfCursorPainter.h"
#include "PdfRegularDocTableModel.h"
#include "Content.h"
#include "Diff.h"

using namespace PoDoFo;
using namespace TR::XML;

namespace TR { namespace Core {

namespace
{
    struct ValidationError
    {
        std::wstring m_name;
        std::wstring m_value;
        std::wstring m_comment;
    };

    struct ValidationErrorCollector : boost::static_visitor<void>
    {
        ValidationErrorCollector(std::vector<ValidationError>& errors, std::wstring path) :
            m_errors(errors),
            m_path(std::move(path))
        {
        }

        void operator ()(const XmlStructNode& struct_node)
        {
            auto& def_node = struct_node.get_bound_def();
            std::wstring path = m_path;
            path += L"/";
            path += def_node.get_caption();
            if (auto id_attr = struct_node.find_id_attr())
            {
                path += L"[";
                path += xml_tools::get_attr_wvalue(*id_attr);
                path += L"]";
            }

            for (auto& regular_node : struct_node.get_members())
            {
                visit_regular_node(ValidationErrorCollector(m_errors, path), regular_node);
            }
        }

        template<class T>
        void operator ()(const XmlListNode<T>& list_node)
        {
            auto& def_node = list_node.get_bound_def();
            std::wstring path = m_path;
            path += L"/";
            path += def_node.get_caption();

            for (auto& regular_node : list_node.get_items())
            {
                visit_regular_node(ValidationErrorCollector(m_errors, path), regular_node);
            }
        }

        template<class T>
        void operator ()(const XmlTrivialNode<T>& trivial_node)
        {
            auto& validated_node = static_cast<const XmlValidatedNode&>(static_cast<const XmlRegularNode&>(trivial_node));
            if (validated_node.is_valid())
            {
                return;
            }

            auto& def_node = trivial_node.get_bound_def();
            std::wstring path = m_path;
            path += L"/";
            path += def_node.get_caption();

            m_errors.push_back({ std::move(path), trivial_node.get_wtext(), validated_node.get_comment() });
        }

        std::vector<ValidationError>& m_errors;
        std::wstring m_path;
    };

    std::vector<ValidationError> GetValidationErrors(const XmlRegularDoc& regDoc)
    {
        std::vector<ValidationError> errors;
        visit_regular_node(ValidationErrorCollector(errors, L"/"), *regDoc.get_root());
        return errors;
    }
}

const char* TITLE_FONT_NAME = "Arial";
const double TITLE_FONT_SIZE = 20.0;
const char* TITLE_2_FONT_NAME = "Arial";
const double TITLE_2_FONT_SIZE = 16.0;
const char* TITLE_3_FONT_NAME = "Arial";
const double TITLE_3_FONT_SIZE = 14.0;
const char* TEXT_FONT_NAME = "Arial";
const double TEXT_FONT_SIZE = 12.0;
const char* TABLE_FONT_NAME = "Arial";
const double TABLE_FONT_SIZE = 10.0;
const double TABLE_ROW_HEIGHT = 20.0;
const double LEFT_MARGIN = 20.0;
const double LEFT_MARGIN_2 = LEFT_MARGIN + 10.0;
const double INDENT = 10.0;
const double INDENT_2 = INDENT + 5.0;

PdfPage* pageBreak(PdfRect& rClipRect, void* pCustom)
{
    auto page = ((PdfStreamedDocument*)pCustom)->CreatePage(PdfPage::CreateStandardPageSize(ePdfPageSize_A4));
    rClipRect.SetHeight(A4_HEIGHT - TOP_MARGIN - BOTTOM_MARGIN);
    rClipRect.SetBottom(BOTTOM_MARGIN);
    return page;
};

SummaryPdf::SummaryPdf() = default;

SummaryPdf::SummaryPdf(const std::wstring& path):
    m_pdf(std::make_unique<PdfStreamedDocument>(path.c_str())),
    m_painter(std::make_unique<PdfCursorPainter>(*m_pdf))
{
    m_pdf->GetInfo()->SetCreator(PdfString("examplahelloworld - A PoDoFo test application"));
    m_pdf->GetInfo()->SetAuthor(PdfString("Dominik Seichter"));
    m_pdf->GetInfo()->SetTitle(PdfString("Hello World"));
    m_pdf->GetInfo()->SetSubject(PdfString("Testing the PoDoFo PDF Library"));
    m_pdf->GetInfo()->SetKeywords(PdfString("Test;PDF;Hello World;"));
}

SummaryPdf::SummaryPdf(const std::ostream* ostream):
    m_output(std::make_unique<PdfOutputDevice>(ostream)),
    m_pdf(std::make_unique<PdfStreamedDocument>(m_output.get())),
    m_painter(std::make_unique<PdfCursorPainter>(*m_pdf))
{
    m_pdf->GetInfo()->SetCreator(PdfString("examplahelloworld - A PoDoFo test application"));
    m_pdf->GetInfo()->SetAuthor(PdfString("Dominik Seichter"));
    m_pdf->GetInfo()->SetTitle(PdfString("Hello World"));
    m_pdf->GetInfo()->SetSubject(PdfString("Testing the PoDoFo PDF Library"));
    m_pdf->GetInfo()->SetKeywords(PdfString("Test;PDF;Hello World;"));
}

SummaryPdf::~SummaryPdf()
{
    if (m_painter)
    {
        close();
    }
}

void SummaryPdf::connect(const std::ostream* ostream)
{
    _ASSERT(m_output == nullptr && m_pdf == nullptr && m_painter == nullptr);

    m_output = std::make_unique<PdfOutputDevice>(ostream);
    m_pdf = std::make_unique<PdfStreamedDocument>(m_output.get());
    m_painter = std::make_unique<PdfCursorPainter>(*m_pdf);

    m_pdf->GetInfo()->SetCreator(PdfString("examplahelloworld - A PoDoFo test application"));
    m_pdf->GetInfo()->SetAuthor(PdfString("Dominik Seichter"));
    m_pdf->GetInfo()->SetTitle(PdfString("Hello World"));
    m_pdf->GetInfo()->SetSubject(PdfString("Testing the PoDoFo PDF Library"));
    m_pdf->GetInfo()->SetKeywords(PdfString("Test;PDF;Hello World;"));
}

void SummaryPdf::print_overview_title()
{    
    m_painter->SetFont(TITLE_FONT_NAME, TITLE_FONT_SIZE);
    m_painter->DrawText(LEFT_MARGIN, "Overview");
}

void SummaryPdf::print_source_counters(const SourceCounters& source_counters)
{    
    m_painter->SetFont(TITLE_FONT_NAME, TITLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN, "Sources overview");

    PdfSimpleTableModel sourcesModel(2, 4);
    sourcesModel.SetText(0, 0, "Counter");
    sourcesModel.SetText(1, 0, "Value");
    sourcesModel.SetText(0, 1, "Total");
    sourcesModel.SetText(1, 1, std::to_string(source_counters.m_count));
    sourcesModel.SetText(0, 2, "Warnings");
    sourcesModel.SetText(1, 2, std::to_string(source_counters.m_warnings));
    sourcesModel.SetText(0, 3, "Errors");
    sourcesModel.SetText(1, 3, std::to_string(source_counters.m_errors));
    sourcesModel.SetForegroundColor(PdfColor(0, 0, 0));
    sourcesModel.SetWordWrapEnabled(true);

    PdfTable sourcesTable(2, 4);
    sourcesTable.SetRowHeight(TABLE_ROW_HEIGHT);
    sourcesTable.SetAutoPageBreak(true, pageBreak, m_pdf.get());
    sourcesTable.SetModel(&sourcesModel);

    m_painter->SetFont(TABLE_FONT_NAME, TABLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawTable(LEFT_MARGIN_2, sourcesTable);
}

void SummaryPdf::print_report_counters(const ReportCounters& report_counters)
{
    m_painter->SetFont(TITLE_FONT_NAME, TITLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN, "Reports overview");

    PdfSimpleTableModel reportsModel(2, 5);
    reportsModel.SetText(0, 0, "Counter");
    reportsModel.SetText(1, 0, "Value");
    reportsModel.SetText(0, 1, "Total");
    reportsModel.SetText(1, 1, std::to_string(report_counters.m_count));
    reportsModel.SetText(0, 2, "Integrity checking errors");
    reportsModel.SetText(1, 2, std::to_string(report_counters.m_intrgrity_checking_errors));
    reportsModel.SetText(0, 3, "Compliance errors");
    reportsModel.SetText(1, 3, std::to_string(report_counters.m_compliance_errors));
    reportsModel.SetText(0, 4, "Validation errors");
    reportsModel.SetText(1, 4, std::to_string(report_counters.m_validation_errors));
    reportsModel.SetForegroundColor(PdfColor(0, 0, 0));
    reportsModel.SetWordWrapEnabled(true);
    
    PdfTable reportsTable(2, 5);
    reportsTable.SetRowHeight(20);
    reportsTable.SetAutoPageBreak(true, pageBreak, m_pdf.get());
    reportsTable.SetModel(&reportsModel);

    m_painter->SetFont(TABLE_FONT_NAME, TABLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawTable(LEFT_MARGIN_2, reportsTable);
}

std::wstring get_message(const Alert& alert)
{
    switch (alert.m_reason)
    {
    case Alert::Reason::INTEGRITY_CHECKING_FAILED:
        return L"Integrity checking failed";

    case Alert::Reason::COMPLIANCE_FAILED:
        return L"Compliance checking failed";

    case Alert::Reason::VALIDATION_FAILED:
        return L"Validation failed";

    case Alert::Reason::LOADING_FAILED:
        return L"Loading failed";

    default:
        _ASSERT(false);
    }
    return L"";
}

void SummaryPdf::print_alerts(const std::vector<Alert>& alerts)
{
    m_painter->SetFont(TITLE_FONT_NAME, TITLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN, "Alerts");

    PdfSimpleTableModel alertsModel(4, int(alerts.size() + 1));
    alertsModel.SetText(0, 0, "Severity");
    alertsModel.SetText(1, 0, "Source");
    alertsModel.SetText(2, 0, "Report");
    alertsModel.SetText(3, 0, "Message");
    alertsModel.SetWordWrapEnabled(true);

    int alert_row = 0;
    for (auto& alert : alerts)
    {
        ++alert_row;
        alertsModel.SetText(0, alert_row, "Warning");
        alertsModel.SetText(1, alert_row, stl_tools::ucs_to_ansi(alert.m_source_name));
        alertsModel.SetText(2, alert_row, stl_tools::ucs_to_ansi(alert.m_report_name));
        alertsModel.SetText(3, alert_row, stl_tools::ucs_to_ansi(get_message(alert)));
        alertsModel.SetForegroundColor(PdfColor(0, 0, 0));
    }

    PdfTable alertsTable(4, int(alerts.size() + 1));
    alertsTable.SetRowHeight(20);
    alertsTable.SetAutoPageBreak(true, pageBreak, m_pdf.get());
    alertsTable.SetModel(&alertsModel);

    m_painter->SetFont(TABLE_FONT_NAME, TABLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawTable(LEFT_MARGIN_2, alertsTable);    
}

void SummaryPdf::print_details_title()
{
    m_painter->SetFont(TITLE_FONT_NAME, TITLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN, "Sources");
}

void SummaryPdf::print_source_title(const std::wstring& source_name, const std::wstring& source_type)
{
    m_painter->SetFont(TITLE_2_FONT_NAME, TITLE_2_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN, 
        stl_tools::ucs_to_ansi(source_name) + 
        " (" + stl_tools::ucs_to_ansi(source_type) + ")");
}

void SummaryPdf::print_source_config(std::shared_ptr<const XmlPropertiesDoc> config)
{
    m_painter->SetFont(TITLE_3_FONT_NAME, TITLE_3_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN, "Settings");

    XmlRegularDocPdfTableModel settingsTableModel(*config, true);
    PdfTable settingsTable(2, int(settingsTableModel.GetSize()));
    settingsTable.SetRowHeight(20);
    settingsTable.SetAutoPageBreak(true, pageBreak, m_pdf.get());
    settingsTable.SetModel(&settingsTableModel);

    m_painter->SetFont(TABLE_FONT_NAME, TABLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawTable(LEFT_MARGIN_2, settingsTable);
}

void SummaryPdf::print_source_issues_title()
{
    m_painter->SetFont(TITLE_3_FONT_NAME, TITLE_3_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN, "Issues");
}

void SummaryPdf::print_source_has_no_issues()
{
    m_painter->SetFont(TEXT_FONT_NAME, TEXT_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN_2, "Source has no issues");
}

std::string diff_state_to_string(DiffState diff_state)
{
    switch (diff_state)
    {
    case DIFF_ADD:
        return "Add";

    case DIFF_DELETE:
        return "Remove";

    default:
        _ASSERT(false);
    }

    return "";
}

void SummaryPdf::print_integrity_checking_alert(const std::wstring& report_name, std::shared_ptr<const Diff> diff)
{
    m_painter->SetFont(TEXT_FONT_NAME, TEXT_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN_2, "Integrity checking failed for " + 
        stl_tools::ucs_to_ansi(report_name));

    if (auto plain_text_diff = std::dynamic_pointer_cast<const PlainTextDiff>(diff))
    {
        auto& diff_lines = plain_text_diff->get_diff_lines();
        PdfSimpleTableModel diffModel(3, int(diff_lines.size() + 1));
        diffModel.SetWordWrapEnabled(true);
        diffModel.SetForegroundColor(PdfColor(0, 0, 0));
        diffModel.SetText(0, 0, "Operation");
        diffModel.SetText(1, 0, "Position");
        diffModel.SetText(2, 0, "Value");

        for (int i = 0; i < diff_lines.size(); ++i)
        {
            auto& diff_line = diff_lines[i];
            diffModel.SetText(0, i + 1, diff_state_to_string(diff_line.m_diff_state));
            diffModel.SetText(1, i + 1, std::to_string(diff_line.m_line_pos));           
            diffModel.SetText(2, i + 1, stl_tools::ucs_to_ansi(diff_line.m_line_str));
        }

        m_painter->SetFont(TABLE_FONT_NAME, TABLE_FONT_SIZE);
        m_painter->MoveCursor(INDENT);

        PdfTable diffTable(3, int(diff_lines.size() + 1));
        auto colWidths = GetColumnWidths({ 100, 50, 0 }, LEFT_MARGIN_2); 
        diffTable.SetColumnWidths(&colWidths[0]);
        auto rowHeights = GetRowHeights(diffModel, diff_lines.size() + 1, colWidths, *m_painter->GetFont());        
        diffTable.SetRowHeights(&rowHeights[0]);
        diffTable.SetAutoPageBreak(true, pageBreak, m_pdf.get());
        diffTable.SetModel(&diffModel);

        m_painter->DrawTable(LEFT_MARGIN_2, diffTable);
    }
}

void SummaryPdf::print_validation_alert(const std::wstring& report_name, const XmlRegularDoc& validationDoc)
{
    m_painter->SetFont(TEXT_FONT_NAME, TEXT_FONT_SIZE);
    m_painter->MoveCursor(INDENT);
    m_painter->DrawText(LEFT_MARGIN_2, "Validation errors found for " +
        stl_tools::ucs_to_ansi(report_name));

    auto validation_errors = GetValidationErrors(validationDoc);
    PdfSimpleTableModel errors_model(3, int(validation_errors.size() + 1));
    errors_model.SetWordWrapEnabled(true);
    errors_model.SetForegroundColor(PdfColor(0, 0, 0));
    errors_model.SetText(0, 0, "Path");
    errors_model.SetText(1, 0, "Value");
    errors_model.SetText(2, 0, "Comment");
    
    for (int i = 0; i < validation_errors.size(); ++i)
    {
        auto& validation_error = validation_errors[i];
        errors_model.SetText(0, i + 1, stl_tools::ucs_to_ansi(validation_error.m_name));
        errors_model.SetText(1, i + 1, stl_tools::ucs_to_ansi(validation_error.m_value));
        errors_model.SetText(2, i + 1, stl_tools::ucs_to_ansi(validation_error.m_comment));
    }

    m_painter->SetFont(TABLE_FONT_NAME, TABLE_FONT_SIZE);
    m_painter->MoveCursor(INDENT);

    PdfTable diffTable(3, int(validation_errors.size() + 1));
    auto colWidths = GetColumnWidths({ 100, 50, 0 }, LEFT_MARGIN_2);
    diffTable.SetColumnWidths(&colWidths[0]);
    auto rowHeights = GetRowHeights(errors_model, validation_errors.size() + 1, colWidths, *m_painter->GetFont());
    diffTable.SetRowHeights(&rowHeights[0]);
    diffTable.SetAutoPageBreak(true, pageBreak, m_pdf.get());
    diffTable.SetModel(&errors_model);

    m_painter->DrawTable(LEFT_MARGIN_2, diffTable);
}

void SummaryPdf::close()
{
    m_painter.reset();
    m_pdf->Close();
    m_pdf.reset();
    m_output->Flush();
    m_output.reset();
}

}} //namespace TR { namespace Core {
