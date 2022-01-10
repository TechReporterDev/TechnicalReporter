#include "stdafx.h"
#include "RunTaskWizard.h"
#include "PropertyDialog.h"
#include "CommonPropertyPage.h"
#include "CommonPropertyDoc.h"
#include "ContainerArray.h"
#include "ReloadDlg.h"
#include "ActionDlg.h"
#include "ShortcutDlg.h"
#include "Application.h"

class RunTaskWizard::ActionParamsDlg: public PropertyDialog
{
public:
    ActionParamsDlg(RunTaskWizard& parent):
        PropertyDialog(L"Action parameters"),
        m_parent(parent),
        m_params(m_parent.m_params->clone())
    {
        m_params = m_parent.m_params->clone();
        init(std::make_unique<CommonPropertyPage>(
            createProperty(*m_params.m_params_doc))
        );
    }

protected:
    // Dialog override
    virtual void apply()
    {
        m_parent.m_params = std::move(m_params);
    }

private:
    RunTaskWizard& m_parent;
    TR::Params m_params;
};

class RunTaskWizard::ActionParamsPage: public CommonPropertyPage
{
public:
    ActionParamsPage(RunTaskWizard& parent):
        CommonPropertyPage(L"Action parameters", L"Setup action parameters", RunTaskWizard::ACTION_PARAMS),
        m_parent(parent)
    {
        show(CommonProperty(L"Action parameters", L"Setup action parameters",
            SwitchPropertyEx(L"", L"",
                {{L"Use custom parameters", 1}, {L"Use shortcut", boost::none}},
                refValue(m_parent.m_useShortcut)
            ))
        );      
    }

protected:
    // CommonPropertyPage override
    virtual void handleEvent(boost::any) override
    {
        auto paramsDlg = new ActionParamsDlg(m_parent);
        paramsDlg->show();
    }

private:
    RunTaskWizard& m_parent;
};

RunTaskWizard::RunTaskWizard(Wt::WWidget* parent):
    PropertyWizard("Run task wizard"),
    m_client(Application::instance()->getClient()),
    m_kindOfTask(RELOAD),
    m_selectedReport(0),
    m_selectedAction(0),
    m_useShortcut(0),
    m_selectedShortcut(0)
{
    init(createKindOfTaskPage());
}

std::unique_ptr<PropertyPageWidget> RunTaskWizard::createKindOfTaskPage()
{
    return std::make_unique<CommonPropertyPage>(CommonProperty{L"Kind of task", L"Select what kind of task you want to launch",
        SwitchProperty(L"", L"",
            {L"Reload report", L"Launch action or shortcut"},
            refValue(m_kindOfTask)
        )
    }, KIND_OF_TASK);
}

std::unique_ptr<PropertyPageWidget> RunTaskWizard::createSelectReportPage()
{
    if (!m_reportTypes)
    {
        m_reportTypes = m_client.getReportTypesInfo();
        m_selectedReport = m_reportTypes->empty() ? -1 : 0;
    }

    return std::make_unique<CommonPropertyPage>(CommonProperty{L"Reload", L"Select report to reload",
        SelectionListProperty(L"Report name", L"",
            stl_tools::copy_vector(*m_reportTypes | stl_tools::members(&TR::ReportTypeInfo::m_name)),
            refValue(m_selectedReport),
            SELECTIONLIST_ATTRIBUTES(SELECTION_REQUIRED)
        )
    }, SELECT_REPORT);
}

std::unique_ptr<PropertyPageWidget> RunTaskWizard::createSelectActionPage()
{
    if (!m_actions)
    {
        m_actions = m_client.getActionsInfo();
        m_selectedAction = m_actions->empty() ? -1 : 0;
    }

    return std::make_unique<CommonPropertyPage>(CommonProperty{L"Action", L"Select action to launch",
        SelectionListProperty(L"Action name", L"", 
            stl_tools::copy_vector(*m_actions | stl_tools::members(&TR::ActionInfo::m_name)),
            customValue<int>(
                [&]{ return m_selectedAction; },
                [&](int position){
                    m_selectedAction = position;
                    m_params.reset();
                }),
            SELECTIONLIST_ATTRIBUTES(SELECTION_REQUIRED)
        )
    }, SELECT_ACTION);
}

std::unique_ptr<PropertyPageWidget> RunTaskWizard::createActionParamsPage()
{
    if (!m_params)
    {
        auto& selectedAction = (*m_actions)[m_selectedAction];
        m_params = m_client.getDefaultParams(selectedAction.m_uuid);
    }

    return std::make_unique<ActionParamsPage>(*this);
}

std::unique_ptr<PropertyPageWidget> RunTaskWizard::createSelectShortcutPage()
{
    auto& selectedAction = (*m_actions)[m_selectedAction];
    m_selectedShortcut = selectedAction.m_shortcuts.empty()? -1: 0;

    return std::make_unique<CommonPropertyPage>(CommonProperty{L"Shortcut", L"Select shortcut to launch",
        SelectionListProperty(L"Shortcut name", L"", 
            stl_tools::copy_vector(selectedAction.m_shortcuts | stl_tools::members(&TR::ActionShortcutInfo::m_name)),
            refValue(m_selectedShortcut),
            SELECTIONLIST_ATTRIBUTES(SELECTION_REQUIRED)
        )
    }, SELECT_SHORTCUT);
}

std::unique_ptr<PropertyPageWidget> RunTaskWizard::createSelectSourcePage()
{
    if (!m_sources)
    {
        m_sources = m_client.getSourcesInfo();
        m_sourceTypes = m_client.getSourceTypesInfo();
    }

    m_targets.clear();
    for (auto& sourceInfo : *m_sources)
    {
        if (m_kindOfTask == KindOfTask::RELOAD)
        {
            auto& selectedReport = (*m_reportTypes)[m_selectedReport];
            for (auto& sourceReport : sourceInfo.m_current_reports)
            {
                if (sourceReport.m_report_type_uuid == selectedReport.m_uuid)
                {
                    m_targets.push_back(sourceInfo);
                    break;
                }
            }
        }
        else if (m_kindOfTask == KindOfTask::LAUNCH)
        {
            auto& selectedAction = (*m_actions)[m_selectedAction];
            for (auto& sourceType : *m_sourceTypes)
            {
                if (sourceInfo.m_source_type_uuid == sourceType.m_uuid)
                {
                    for (auto& actionUUID : sourceType.m_action_uuids)
                    {
                        if (actionUUID == selectedAction.m_uuid)
                        {
                            m_targets.push_back(sourceInfo);
                            break;
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            _ASSERT(false);
        }
    }

    m_checkedTargets.clear();
    m_checkedTargets.resize(m_targets.size(), false);

    return std::make_unique<CommonPropertyPage>(CommonProperty{L"Sources", L"Select target sources for task",
        CheckListProperty(L"Source name", L"", 
            stl_tools::copy_vector(m_targets | stl_tools::members(&TR::SourceInfo::m_name)),
            containerArray(m_checkedTargets),
            CHECKLIST_ATTRIBUTES(CHECK_REQUIRED)
        )
    }, RunTaskWizard::TARGET_SOURCES);
}

std::unique_ptr<PropertyPageWidget> RunTaskWizard::createTaskReadyPage()
{
    return std::make_unique<CommonPropertyPage>(CommonProperty{L"Last step", L"Click 'Finish' to run task batch."}, RunTaskWizard::TASK_READY);
}

std::unique_ptr<PropertyPageWidget> RunTaskWizard::nextPage(int currentPageIdentity)
{
    switch (currentPageIdentity)
    {
    case KIND_OF_TASK:
        switch (m_kindOfTask)
        {
        case RELOAD:
            return createSelectReportPage();

        case LAUNCH:
            return createSelectActionPage();

        default:
            _ASSERT(false);
        }
        break;

    case SELECT_REPORT:
        return createSelectSourcePage();

    case SELECT_ACTION:
        return createActionParamsPage();

    case ACTION_PARAMS:
        if (m_useShortcut != 0)
        {
            return createSelectShortcutPage();
        }
        else
        {
            return createSelectSourcePage();
        }

    case SELECT_SHORTCUT:
        return createSelectSourcePage();

    case TARGET_SOURCES:
        return createTaskReadyPage();

    default:
        _ASSERT(false);
    }
    return nullptr;
}

bool RunTaskWizard::isFinal(int currentPageIdentity)
{
    return currentPageIdentity == TASK_READY;
}

void RunTaskWizard::onFinish()
{
    std::vector<TR::SourceInfo> targets;
    for (size_t position = 0; position < m_checkedTargets.size(); ++position)
    {
        if (m_checkedTargets[position])
        {
            targets.push_back(m_targets[position]);
        }
    }

    switch (m_kindOfTask)
    {
        case RELOAD:
        {
            auto& selectedReport = (*m_reportTypes)[m_selectedReport];
            auto reloadDlg = new ReloadBundleDlg(targets, selectedReport);
            reloadDlg->show();
            break;
        }

        case LAUNCH:
        {
            if (m_useShortcut)
            {
                auto& selectedAction = (*m_actions)[m_selectedAction];
                auto& selectedShortcut = selectedAction.m_shortcuts[m_selectedShortcut];
                auto shortcutDlg = new ShortcutBundleDlg(targets, selectedShortcut);
                shortcutDlg->show();
            }
            else
            {
                auto& selectedAction = (*m_actions)[m_selectedAction];
                auto actionDlg = new ActionBundleDlg(targets, selectedAction, std::move(*m_params));
                actionDlg->show();
            }
            break;
        }

        default:
            _ASSERT(false);
    }
}