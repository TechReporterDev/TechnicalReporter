#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class SubjectStreamOptionsWidget: public MainFrameWidget
{
public:
    SubjectStreamOptionsWidget(TR::SubjectStreamOptions subjectStreamOptions);

private:
    void createToolBar();
    void onOk();
    void onCancel();

    TR::SubjectStreamOptions m_subjectStreamOptions;
};

class SubjectStreamOptionsPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    SubjectStreamOptionsPage(TR::SubjectStreamOptions subjectStreamOptions);
    virtual MainFrameWidget*    getWidget() override;

private:
    SubjectStreamOptionsWidget m_widget;
};