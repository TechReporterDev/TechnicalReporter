#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class ActionSubjectOptions;
class ModifiedActionOptions;

class ActionOptionsWidget: public MainFrameWidget
{
public:
    ActionOptionsWidget(const TR::ActionInfo& actionInfo, boost::optional<TR::UUID> shortcutUUID = boost::none);

private:
    void createToolBar();
    virtual void load() override;

    ActionSubjectOptions* m_sourceOptions;
    ActionSubjectOptions* m_roleOptions;
    ModifiedActionOptions* m_modifiedOptions;

    TR::ActionInfo m_actionInfo;
    boost::optional<TR::UUID> m_shortcutUUID;
    std::map<TR::SourceKey, TR::SourceInfo> m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo> m_rolesInfo;
};

class ActionOptionsPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    ActionOptionsPage(const TR::ActionInfo& actionInfo, boost::optional<TR::UUID> shortcutUUID = boost::none);
    virtual MainFrameWidget*    getWidget() override;

private:
    TR::ActionInfo m_actionInfo;
    boost::optional<TR::UUID> m_shortcutUUID;
    std::unique_ptr<ActionOptionsWidget> m_widget;
};