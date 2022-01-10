#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class SourceInfoPanel;
class AlertsPanel;
class CurrentReportPanel;

class SourceWidget: public MainFrameWidget
{
public:
    SourceWidget(TR::SourceKey sourceKey);

private:
    void onReloadClick();
    void onReloadReportClick(TR::ReportTypeInfo reportTypeInfo);
    void onActionClick(TR::ActionInfo actionInfo);
    void onShortcutClick(TR::ActionShortcutInfo shortcutInfo);
    void createToolBar();
    void createActionMenu(Wt::WPopupMenu* actionPopup);
    virtual void load() override;

    TR::SourceInfo m_sourceInfo;
    SourceInfoPanel* m_infoPanel;
    AlertsPanel* m_alertsPanel;
    CurrentReportPanel* m_currentReportPanel;
};

class SourcePage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    SourcePage(TR::SourceInfo sourceInfo);
    virtual MainFrameWidget* getWidget() override;  

private:
    TR::SourceInfo m_sourceInfo;
    std::unique_ptr<SourceWidget> m_widget;
};