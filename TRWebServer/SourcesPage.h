#pragma once
#include "SourcesView.h"
#include "TRWebServer.h"
#include "MainFrame.h"

class SourcesWidget: public MainFrameWidget
{
public:
    enum Mode { TABLE_MODE, TREE_MODE };
    SourcesWidget();
    
protected:
    // AppEventHandler override
    virtual void onAddSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;
    virtual void onRemoveSource(TR::Key, const std::vector<TR::SourceKey>& removedSources) override;
    virtual void onAddLink(TR::Link link) override;
    virtual void onRemoveLink(TR::Link link) override;

private:
    void createToolBar();
    void onAddClick();
    void onRemoveClick();
    void onEditClick();
    void onRunClick();  
    void onSourceDblClick(const SourcesView::Item& item);
    void setMode(Mode mode);
    
    SourcesView* m_sourcesView;
    std::map<TR::UUID, TR::SourceTypeInfo> m_sourceTypes;
    std::map<TR::SourceKey, TR::SourceInfo> m_sources;
};

class SourcesPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    SourcesPage();
    virtual MainFrameWidget* getWidget() override;

private:
    std::unique_ptr<SourcesWidget> m_widget;
};