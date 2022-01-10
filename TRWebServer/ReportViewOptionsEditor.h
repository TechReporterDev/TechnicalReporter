#pragma once
#include "PropertySheet.h"
#include "TRWebServer.h"
#include "MainFrame.h"

class ReportViewOptionsEditor: public PropertySheetWidget
{
public:
    ReportViewOptionsEditor(TR::SubjectReportOptions subjectReportOptions);
    virtual bool apply() override;

private:
    class GeneralPage;

    PropertyPageWidget* createGeneralPage();

    TR::SubjectReportOptions m_subjectReportOptions;
    boost::optional<TR::ReportTypeInfo> m_reportTypeInfo;
};