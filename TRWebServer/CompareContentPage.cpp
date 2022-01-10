#include "stdafx.h"
#include "CompareContentPage.h"
#include "Application.h"

CompareContentWidget::CompareContentWidget(const TR::ReportTypeInfo& reportTypeInfo, std::unique_ptr<LazyContentQuery> left, std::unique_ptr<LazyContentQuery> right):
    m_reportTypeInfo(reportTypeInfo),
    m_query({std::move(left), std::move(right)})
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);   

    setDiffView(std::make_unique<EmptyDiffView>("Loading..."));
    m_query.run(
        [&](LazyQueryBatch::Contents& contents){
            onQueryCompleted(contents);
        },
        [&](const std::string&){
            onQueryFailed();
        }
    );  
}

void CompareContentWidget::showDiff(std::shared_ptr<TR::Content> left, std::shared_ptr<TR::Content> right)
{
    if (left == nullptr || right == nullptr)
    {
        setDiffView(std::make_unique<EmptyDiffView>());
    }
    else
    {
        setDiffView(createDiffView(m_reportTypeInfo, left, right));
    }
}

void CompareContentWidget::setDiffView(std::unique_ptr<DiffView> diffView)
{   
    auto vbox = Wt::clear(this);
    showToolBar(diffView.get());    
    vbox->addWidget(std::move(diffView));   
}

void CompareContentWidget::showToolBar(DiffView* diffView)
{
    if (auto viewBar = diffView->getViewBar())
    {
        auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());
        toolbar->addWidget(std::move(viewBar));
    }
}

void CompareContentWidget::onQueryCompleted(LazyQueryBatch::Contents& contents)
{
    showDiff(contents[0], contents[1]); 
}

void CompareContentWidget::onQueryFailed()
{
    setDiffView(std::make_unique<EmptyDiffView>()); 
}

CompareContentPage::CompareContentPage(const TR::ReportTypeInfo& reportTypeInfo, std::unique_ptr<LazyContentQuery> left, std::unique_ptr<LazyContentQuery> right):
    MainFramePage(L"Reports compare", "/compare_content"),
    m_widget(reportTypeInfo, std::move(left), std::move(right))
{
}

MainFrameWidget* CompareContentPage::getWidget()
{
    return &m_widget;
}