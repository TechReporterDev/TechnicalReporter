#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "Panel.h"
class StatsTable;
class StatsPie;
class AlertsTable;
class ChangesTable;

class SummaryPanel: public Panel
{
public:
    SummaryPanel();

private:
    void updateStats();
    void invalidate();

    // AppEventHandler override
    virtual void onAddSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources) override;

    StatsTable* m_statsTable;
    StatsPie* m_statsPie;
    bool m_dirty;
};

class MessagePanel: public Panel
{
public:
    MessagePanel(const SourceInfoByKey& sourcesInfo, const ReportTypeInfoByUUID& reportTypesInfo, const ActionInfoByUUID& actionsInfo, const ShortcutInfoByUUID& shortcutsInfo);

protected:
    // AppEventHandler override
    virtual void onAddSource(const TR::SourceInfo& sourceInfo) override;    
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources) override;

    void addAlerts(const TR::SourceInfo& sourceInfo);
    void removeAlerts(TR::SourceKey sourceKey);

private:
    AlertsTable* m_alertsTable;
    const SourceInfoByKey& m_sourcesInfo;
    const ReportTypeInfoByUUID& m_reportTypesInfo;
    const ActionInfoByUUID& m_actionsInfo;
    const ShortcutInfoByUUID& m_shortcutsInfo;
};

class ChangesPanel: public Panel
{
public: 
    ChangesPanel(const SourceInfoByKey& sourcesInfo, const ReportTypeInfoByUUID& reportTypesInfo, const StreamTypeInfoByUUID& streamTypesInfo);
    
protected:
    // AppEventHandler override
    virtual void onAddSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources) override;

    void addChanges(const TR::SourceInfo& sourceInfo);
    void removeChanges(TR::SourceKey sourceKey);

private:
    ChangesTable* m_changesTable;
    const SourceInfoByKey& m_sourcesInfo;
    const ReportTypeInfoByUUID& m_reportTypesInfo;
    const StreamTypeInfoByUUID& m_streamTypesInfo;
};