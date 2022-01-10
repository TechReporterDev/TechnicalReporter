#include "stdafx.h"
#include "TextDiffView.h"
#include "TextDiffTableView.h"
#include "Application.h"

TextDiffView::TextDiffView(std::shared_ptr<TR::TextContent> oldContent, std::shared_ptr<TR::PlainTextDiff> diff):
    m_oldContent(oldContent),
    m_diff(diff),
    m_tableView(nullptr)
{
    _ASSERT(oldContent && diff);

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    m_tableView = vbox->addWidget(std::make_unique<TextDiffTableView>(m_oldContent->as_wstring(), diff->get_diff_lines()));
}

std::unique_ptr<Wt::WWidget> TextDiffView::getViewBar()
{
    auto toolbar = std::make_unique<Wt::WToolBar>();

    auto prevBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Prev");
    prevBtn->clicked().connect([this](Wt::WMouseEvent) {
        m_tableView->selectPrev();
    }); 

    auto nextBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Next");
    nextBtn->clicked().connect([this](Wt::WMouseEvent) {
        m_tableView->selectNext();  
    });

    return toolbar;
}