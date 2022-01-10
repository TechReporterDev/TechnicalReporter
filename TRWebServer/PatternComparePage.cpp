#include "stdafx.h"
#include "PatternComparePage.h"
#include "Application.h"
#include "PropertyBox.h"

PatternCompareWidget::PatternCompareWidget(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo):
    CompareContentWidget(
        reportTypeInfo,
        std::make_unique<PatternQuery>(&Application::instance()->getClient(), sourceInfo.m_key, reportTypeInfo.m_uuid),
        std::make_unique<CurrentContentQuery>(&Application::instance()->getClient(), sourceInfo.m_key, reportTypeInfo.m_uuid)),
    m_sourceInfo(sourceInfo)
{
}

void PatternCompareWidget::showToolBar(DiffView* diffView)
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    if (auto viewBar = diffView->getViewBar())
    {       
        toolbar->addWidget(std::move(viewBar));
        toolbar->addSeparator();
    }

    auto resetBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Reset pattern");
    resetBtn->clicked().connect([this](Wt::WMouseEvent){
        m_client->resetPattern(m_sourceInfo.m_key, m_reportTypeInfo.m_uuid);        
        auto propertyBox = new PropertyBox(Dialog::BTN_CLOSE, L"Completed", L"Integrity checking pattern was reseted.");
        propertyBox->show();
    }); 
}

PatternComparePage::PatternComparePage(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo):
    MainFramePage(L"Pattern compare", "/pattern_compare"),
    m_widget(sourceInfo, reportTypeInfo)
{
}

MainFrameWidget* PatternComparePage::getWidget()
{
    return &m_widget;
}