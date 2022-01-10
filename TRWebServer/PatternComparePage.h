#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "CompareContentPage.h"

class PatternCompareWidget: public CompareContentWidget
{
public:
    PatternCompareWidget(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo);

protected:
    virtual void showToolBar(DiffView* diffView);

    TR::SourceInfo m_sourceInfo;
};

class PatternComparePage: public MainFramePage
{
public:
    PatternComparePage(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo);
    virtual MainFrameWidget* getWidget() override;

protected:
    PatternCompareWidget m_widget;
};

