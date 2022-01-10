#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "ContentView.h"

class TaskOutputWidget: public MainFrameWidget
{
public:
    TaskOutputWidget(const TR::SourceInfo& sourceInfo, std::shared_ptr<TR::Content> content);
    TaskOutputWidget(const TaskOutputWidget&) = delete;

protected:
    void            showTitleBar();
    void            showContent();
    void            setContentView(std::unique_ptr<ContentView> contentView);
    ContentView*    getContentView();

    TR::SourceInfo m_sourceInfo;
    std::shared_ptr<TR::Content> m_content;
};

class TaskOutputPage: public MainFramePage
{
public:
    TaskOutputPage(const std::wstring& caption, const TR::SourceInfo& sourceInfo, std::shared_ptr<TR::Content> content);
    virtual MainFrameWidget* getWidget() override;

protected:
    std::unique_ptr<TaskOutputWidget> m_widget;
};