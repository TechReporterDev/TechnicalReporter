#include "stdafx.h"
#include "ValidatorEditor.h"
#include "ValidatorView.h"
#include "Application.h"

ValidatorEditor::ValidatorEditor(TR::SubjectKey subjectKey, TR::ValidationKey validationKey):
    m_subjectKey(subjectKey),
    m_validationKey(validationKey)
{

    m_validationSettings = m_client->getValidationSettings(m_subjectKey, m_validationKey);
    
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->addWidget(std::make_unique<ValidatorView>(*m_validationSettings->m_validator, m_validationSettings->m_base_validator.get()), 1);
}

void ValidatorEditor::apply()
{
    m_client->setValidationSettings(m_subjectKey, m_validationKey, m_validationSettings->clone());
}