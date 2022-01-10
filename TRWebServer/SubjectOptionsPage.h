#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class SubjectReportOptions;
class SubjectStreamOptions;
class SubjectActionOptions;
class SubjectModifiedOptions;

class SubjectOptionsWidget: public MainFrameWidget
{
public:
    SubjectOptionsWidget(TR::SubjectKey subjectKey);
    void createToolBar();

private:
    virtual void load() override;

    SubjectReportOptions* m_reportOptions;
    SubjectStreamOptions* m_streamOptions;
    SubjectActionOptions* m_actionOptions;
    SubjectModifiedOptions* m_modifiedOptions;

    TR::SubjectKey m_subjectKey;
};

class SubjectOptionsPage: public MainFramePage
{
public:
    SubjectOptionsPage(std::wstring caption, std::string url);

private:
    std::unique_ptr<SubjectOptionsWidget> m_widget;
};