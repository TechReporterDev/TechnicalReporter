#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "Panel.h"
class CurrentReportsTable;
class AlertsTable;

class SourceInfoPanel: public Panel
{
public:
    SourceInfoPanel(const TR::SourceInfo& sourceInfo);

private:
    void showSourceInfo();
    void addLine(const Wt::WString& label, const Wt::WString& value);

    // AppEventHandler override
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;

    TR::SourceInfo m_sourceInfo;
};

class CurrentReportPanel: public Panel
{
public:
    CurrentReportPanel(const TR::SourceInfo& sourceInfo);
    
private:
    void showCurrentReports();

    // AppEventHandler override
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;
    
    CurrentReportsTable* m_currentReportsTable;
    TR::SourceInfo m_sourceInfo;
};

class AlertsPanel: public Panel
{
public:
    AlertsPanel(const TR::SourceInfo& sourceInfo);

private:
    void showAlerts(const TR::SourceInfo& sourceInfo);

    // AppEventHandler override
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;
    
    AlertsTable* m_alertsTable;
    TR::SourceKey m_sourceKey;
    std::map<TR::ReportTypeUUID, TR::ReportTypeInfo> m_reportTypesInfo;
    std::map<TR::ActionUUID, TR::ActionInfo> m_actionsInfo;
    std::map<TR::UUID, TR::ActionShortcutInfo> m_shortcutsInfo;
};