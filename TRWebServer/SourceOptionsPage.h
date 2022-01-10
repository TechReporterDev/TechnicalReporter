#pragma once
#include "TRWebServer.h"
#include "SubjectOptionsPage.h"

class SourceOptionsWidget: public SubjectOptionsWidget
{
public:
    SourceOptionsWidget(const TR::SourceInfo& sourceInfo);

private:
    TR::SourceInfo m_sourceInfo;
};

class SourceOptionsPage: public SubjectOptionsPage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    SourceOptionsPage(const TR::SourceInfo& sourceInfo);
    virtual MainFrameWidget*    getWidget() override;

private:
    TR::SourceInfo m_sourceInfo;
    std::unique_ptr<SourceOptionsWidget> m_widget;
};