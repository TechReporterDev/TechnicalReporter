#pragma once
#include "ContentEditor.h"
#include "TRWebServer.h"
#include "MainFrame.h"

class ValidatorEditor: public MainFrameWidget
{
public:
    ValidatorEditor(TR::SubjectKey subjectKey, TR::ValidationKey validationKey);
    ValidatorEditor(const ValidatorEditor&) = delete;
    ValidatorEditor& operator = (const ValidatorEditor&) = delete;

    void apply();

protected:
    TR::SubjectKey m_subjectKey;
    TR::ValidationKey m_validationKey;
    boost::optional<TR::ValidationSettings> m_validationSettings;
};