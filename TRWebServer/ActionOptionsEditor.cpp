#include "stdafx.h"
#include "ActionOptionsEditor.h"
#include "OptionProperties.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "StaticValue.h"
#include "CommonPropertyPage.h"
#include "Application.h"

class ActionOptionsEditor::GeneralPage: public CommonPropertyPage
{
public:
    GeneralPage(ActionOptionsEditor* optionsEditor):
        CommonPropertyPage(L"General", L"General action options"),
        m_optionsEditor(optionsEditor),
        m_options(optionsEditor->m_subjectActionOptions)
    {
        CommonProperty properties(L"", L"");
        if (m_options.m_shortcut_uuid)
        {
            auto& client = Application::instance()->getClient();
            m_schedulesInfo = client.getSchedulesInfo();

            properties.addProperties(
                ScheduleProperty(L"Schedule", L"Select schedule", m_schedulesInfo, *m_options.m_default_schedule_key, m_options.m_schedule_key)
            );
        }

        show(std::move(properties));
    }   

private:
    ActionOptionsEditor* m_optionsEditor;
    TR::SubjectActionOptions& m_options;
    std::vector<TR::ScheduleInfo> m_schedulesInfo;
};

ActionOptionsEditor::ActionOptionsEditor(TR::SubjectActionOptions subjectActionOptions):
    m_subjectActionOptions(std::move(subjectActionOptions))
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    auto propertySheet = vbox->addWidget(std::make_unique<PropertySheetWidget>(), 1);
    propertySheet->pushPage(createGeneralPage());
}

bool ActionOptionsEditor::apply()
{
    if (!PropertySheetWidget::apply())
    {
        return false;
    }

    auto& client = Application::instance()->getClient();
    client.setOptions(m_subjectActionOptions.m_subject_key, m_subjectActionOptions);
    return true;
}

std::unique_ptr<PropertyPageWidget> ActionOptionsEditor::createGeneralPage()
{
    return std::make_unique<GeneralPage>(this);
}