#include "stdafx.h"
#include "ValidatedContentView.h"
#include "ValidatedDocTreeView.h"
#include "ValidatedDocModel.h"
#include "ValidatedDocTableView.h"
#include "Application.h"

ValidatedContentView::ValidatedContentView(std::shared_ptr<const TR::RegularContent> content):
    m_content(content),
    m_mode(TREE_MODE)
{
    _ASSERT(content);
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    init();
}

void ValidatedContentView::update(std::shared_ptr<const TR::Content> content)
{
    _ASSERT(content);
    m_content = std::static_pointer_cast<const TR::RegularContent>(content);
    init();
}

std::unique_ptr<Wt::WToolBar> ValidatedContentView::getViewBar()
{
    auto toolbar = std::make_unique<Wt::WToolBar>();

    auto treeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Tree");
    treeBtn->setCheckable(true);
    treeBtn->setChecked(true);

    auto tableBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Table");
    tableBtn->setCheckable(true);
    tableBtn->setChecked(false);

    treeBtn->clicked().connect([tableBtn, this](Wt::WMouseEvent) {
        //m_treeBtn->setChecked(true);
        tableBtn->setChecked(false);
        setMode(ValidatedContentView::Mode::TREE_MODE);
    });

    tableBtn->clicked().connect([treeBtn, this](Wt::WMouseEvent) {
        //m_tableBtn->setChecked(true);
        treeBtn->setChecked(false);
        setMode(ValidatedContentView::Mode::TABLE_MODE);
    });

    return toolbar;
}

void ValidatedContentView::setMode(Mode mode)
{
    m_mode = mode;
    init();
}

void ValidatedContentView::init()
{
    switch (m_mode)
    {
    case Mode::TREE_MODE:
    {
        auto model = std::make_unique<ValidatedDocModel>(const_cast<TR::XML::XmlRegularDoc&>(m_content->as_regular_doc()));
        model->addColumn(std::make_unique<RegularNameColumn>());
        model->addColumn(std::make_unique<RegularValueColumn>());
        model->addColumn(std::make_unique<RegularDescriptionColumn>());
        model->addColumn(std::make_unique<ValidatedCommentColumn>());

        auto treeView = std::make_unique<ValidatedDocTreeView>(std::move(model));       
        treeView->setColumnWidth(0, 300);
        treeView->setColumnWidth(1, 300);
        treeView->setColumnWidth(2, 300);
        treeView->setColumnWidth(3, 300);
        setCurrentView(std::move(treeView));
        break;
    }

    case Mode::TABLE_MODE:
        setCurrentView(std::make_unique<ValidatedDocTableView>(m_content->as_regular_doc()));
        break;

    default:
        _ASSERT(false);
    }
}

Wt::WWidget* ValidatedContentView::getCurrentView()
{
    auto vbox = layout();
    _ASSERT(vbox->count() == 1);
    return vbox->itemAt(0)->widget();
}

void ValidatedContentView::setCurrentView(std::unique_ptr<Wt::WWidget> currentView)
{
    auto vbox = Wt::clear(this);
    vbox->addWidget(std::move(currentView));
}