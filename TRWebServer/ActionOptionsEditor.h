#pragma once
#include "PropertySheet.h"
#include "TRWebServer.h"
#include "MainFrame.h"

class ActionOptionsEditor: public PropertySheetWidget
{
public:
    ActionOptionsEditor(TR::SubjectActionOptions subjectActionOptions);
    virtual bool apply() override;

private:
    class GeneralPage;
    std::unique_ptr<PropertyPageWidget> createGeneralPage();
    TR::SubjectActionOptions m_subjectActionOptions;
};