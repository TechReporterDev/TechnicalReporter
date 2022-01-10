#pragma once
#include "PropertyWizard.h"
class AppClient;

class RunTaskWizard: public PropertyWizard
{
public:
    RunTaskWizard(Wt::WWidget* parent);
    
protected:
    class ActionParamsDlg;
    class ActionParamsPage;

    enum KindOfTask { RELOAD, LAUNCH };
    enum PageID { KIND_OF_TASK = 0, SELECT_REPORT, SELECT_ACTION, ACTION_PARAMS, SELECT_SHORTCUT, TARGET_SOURCES, TASK_READY };
    
    // property pages
    std::unique_ptr<PropertyPageWidget> createKindOfTaskPage();
    std::unique_ptr<PropertyPageWidget> createSelectReportPage();
    std::unique_ptr<PropertyPageWidget> createSelectActionPage();
    std::unique_ptr<PropertyPageWidget> createActionParamsPage();
    std::unique_ptr<PropertyPageWidget> createSelectShortcutPage();
    std::unique_ptr<PropertyPageWidget> createSelectSourcePage();
    std::unique_ptr<PropertyPageWidget> createTaskReadyPage();

    // PropertyWizard override
    virtual std::unique_ptr<PropertyPageWidget> nextPage(int currentPageIdentity) override;
    virtual bool isFinal(int currentPageIdentity) override;
    virtual void onFinish() override;

    AppClient& m_client;
    KindOfTask m_kindOfTask;

    boost::optional<std::vector<TR::ReportTypeInfo>> m_reportTypes;
    int m_selectedReport;

    boost::optional<std::vector<TR::ActionInfo>> m_actions;
    int m_selectedAction;
    int m_useShortcut;
    boost::optional<TR::Params> m_params;
    int m_selectedShortcut;
    
    boost::optional<std::vector<TR::SourceInfo>> m_sources;
    boost::optional<std::vector<TR::SourceTypeInfo>> m_sourceTypes;
    std::vector<TR::SourceInfo> m_targets;
    std::vector<bool> m_checkedTargets; 
};