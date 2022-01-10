#include "stdafx.h"
#include "ReportViewOptionsEditor.h"
#include "OptionProperties.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "StaticValue.h"
#include "CommonPropertyPage.h"
#include "Application.h"

class ReportViewOptionsEditor::GeneralPage: public CommonPropertyPage
{
public:
    GeneralPage(ReportViewOptionsEditor* optionsEditor):
        CommonPropertyPage(L"General", L"General report options"),
        m_options(optionsEditor->m_subjectReportOptions)
    {
        CommonProperty properties(L"", L"", CommonProperty(L"Report Generating", L"",
            PolicyProperty(L"Visible", L"Setup report visibility",
                {L"Visible", L"Hidden"}, m_options.m_report_generating.m_default_visibile,
                m_options.m_report_generating.m_report_visibility
            )
        ));

        show(std::move(properties));
    }

private:
    TR::SubjectReportOptions& m_options;
};

ReportViewOptionsEditor::ReportViewOptionsEditor(TR::SubjectReportOptions subjectReportOptions):
    m_subjectReportOptions(std::move(subjectReportOptions))
{
    auto& client = Application::instance()->getClient();
    m_reportTypeInfo = client.getReportTypeInfo(m_subjectReportOptions.m_report_type_uuid);

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    auto propertySheet = vbox->addWidget(std::make_unique<PropertySheetWidget>(), 1);
    propertySheet->pushPage(std::make_unique<GeneralPage>(this));
}

bool ReportViewOptionsEditor::apply()
{
    if (!PropertySheetWidget::apply())
    {
        return false;
    }

    auto& client = Application::instance()->getClient();
    client.setOptions(m_subjectReportOptions.m_subject_key, m_subjectReportOptions);
    return true;
}