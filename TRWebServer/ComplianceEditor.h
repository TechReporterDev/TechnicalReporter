#pragma once
#include "ContentEditor.h"
#include "TRWebServer.h"
#include "MainFrame.h"

class ComplianceEditor: public MainFrameWidget
{
public:
    ComplianceEditor(TR::SubjectKey subjectKey, TR::ComplianceKey complianceKey);
    ComplianceEditor(const ComplianceEditor&) = delete;
    ComplianceEditor& operator = (const ComplianceEditor&) = delete;

    void apply();

protected:
    TR::SubjectKey m_subjectKey;
    TR::ComplianceKey m_complianceKey;
    boost::optional<TR::ComplianceSetup> m_complianceSetup;
    std::shared_ptr<TR::CheckListContent> m_browseContent;
};