#include "stdafx.h"
#include "ComplianceEditor.h"
#include "ComplianceSetupView.h"
#include "Application.h"

ComplianceEditor::ComplianceEditor(TR::SubjectKey subjectKey, TR::ComplianceKey complianceKey):
    m_subjectKey(subjectKey),
    m_complianceKey(complianceKey)
{
    auto complianceInfo = m_client->getComplianceInfo(m_complianceKey);
    m_complianceSetup = m_client->getComplianceSetup(m_subjectKey, m_complianceKey);

    if (auto sourceKey = boost::get<TR::SourceKey>(&m_subjectKey))
    {
        m_browseContent = std::dynamic_pointer_cast<TR::CheckListContent>(m_client->getCurrentContent(*sourceKey, complianceInfo.m_base_report_type_uuid));
    }
    
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->addWidget(std::make_unique<ComplianceSetupView>(
        *m_complianceSetup->m_setup, 
        *m_complianceSetup->m_base_setup,
        m_browseContent ? &m_browseContent->get_compliance_doc() : nullptr), 1);
}

void ComplianceEditor::apply()
{
    m_client->setComplianceSetup(m_subjectKey, m_complianceKey, m_complianceSetup->clone());
}