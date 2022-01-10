#pragma once
#include "PropertySheet.h"
#include "TRWebServer.h"
#include "MainFrame.h"
class ContentView;
class PatternEditor;
class ComplianceEditor;
class ValidatorEditor;

class ReportOptionsEditor: public PropertySheetWidget
{
public:
    ReportOptionsEditor(TR::SubjectReportOptions subjectReportOptions);
    virtual ~ReportOptionsEditor();

    virtual bool apply() override;

private:
    class GeneralPage;  

    std::unique_ptr<PropertyPageWidget> createGeneralPage();
    std::unique_ptr<PropertyPageWidget> createDefaultPatternPage();
    std::unique_ptr<PropertyPageWidget> createPatternPage();
    std::unique_ptr<PropertyPageWidget> createCompliancePage();
    std::unique_ptr<PropertyPageWidget> createValidationPage();

    std::unique_ptr<ContentView> m_defaultPatternView;
    std::unique_ptr<PatternEditor> m_patternEditor;
    std::unique_ptr<ComplianceEditor> m_complianceEditor;
    std::unique_ptr<ValidatorEditor> m_validatorEditor;

    TR::SubjectReportOptions m_subjectReportOptions;
    boost::optional<TR::ReportTypeInfo> m_reportTypeInfo;   
};