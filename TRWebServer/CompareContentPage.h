#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "DiffView.h"

class CompareContentWidget: public MainFrameWidget
{
public:
    CompareContentWidget(const TR::ReportTypeInfo& reportTypeInfo, std::unique_ptr<LazyContentQuery> left, std::unique_ptr<LazyContentQuery> right);

protected:
    void                    showDiff(std::shared_ptr<TR::Content> left, std::shared_ptr<TR::Content> right);
    void                    setDiffView(std::unique_ptr<DiffView> diffView);
    virtual void            showToolBar(DiffView* diffView);

    void                    onQueryCompleted(LazyQueryBatch::Contents& contents);
    void                    onQueryFailed();

    TR::ReportTypeInfo m_reportTypeInfo;
    LazyQueryBatch m_query;
};

class CompareContentPage: public MainFramePage
{
public:
    CompareContentPage(const TR::ReportTypeInfo& reportTypeInfo, std::unique_ptr<LazyContentQuery> left, std::unique_ptr<LazyContentQuery> right);
    virtual MainFrameWidget* getWidget() override;

protected:
    CompareContentWidget m_widget;
};

