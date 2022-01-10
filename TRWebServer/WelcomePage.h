#pragma once
#include "MainFrame.h"
class SummaryPanel;
class MessagePanel;
class ChangesPanel;

class WelcomeWidget: public MainFrameWidget
{
public:
    WelcomeWidget();

protected:
    // AppEventHandler override
    virtual void onAddSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources) override;

private:
    SourceInfoByKey m_sourcesInfo;
    ReportTypeInfoByUUID m_reportTypesInfo;
    StreamTypeInfoByUUID m_streamTypesInfo;
    ActionInfoByUUID m_actionsInfo;
    ShortcutInfoByUUID m_shortcutsInfo;

    SummaryPanel* m_summaryPanel;
    MessagePanel* m_messagePanel;
    ChangesPanel* m_changesPanel;
};

class WelcomePage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    WelcomePage();
    virtual MainFrameWidget* getWidget() override;

private:
    WelcomeWidget m_widget;
};