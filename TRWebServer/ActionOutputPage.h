#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "ContentView.h"

class ActionOutputWidget: public MainFrameWidget
{
public:
    ActionOutputWidget(const TR::SourceInfo& sourceInfo, const TR::ActionInfo& actionInfo, TR::Params params, std::shared_ptr<TR::Content> content);
    ActionOutputWidget(const ActionOutputWidget&) = delete;

protected:
    void showTitleBar();
    void showContent();
    ContentView* setContentView(std::unique_ptr<ContentView> contentView);
    ContentView* getContentView();
    void onPersistClick();

    TR::SourceInfo m_sourceInfo;
    TR::ActionInfo m_actionInfo;
    TR::Params m_params;
    std::shared_ptr<TR::Content> m_content;
};

class ActionOutputPage: public MainFramePage
{
public:
    ActionOutputPage(const TR::SourceInfo& sourceInfo, const TR::ActionInfo& actionInfo, TR::Params params, std::shared_ptr<TR::Content> content);
    virtual MainFrameWidget* getWidget() override;

protected:
    ActionOutputWidget m_widget;
};