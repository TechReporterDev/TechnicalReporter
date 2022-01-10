#include "stdafx.h"
#include "RegularContentView.h"
#include "RegularDocTreeView.h"
#include "RegularDocModel.h"
#include "RegularDocTableView.h"
#include "QueryDocView.h"
#include "Application.h"

RegularContentView::RegularContentView(std::shared_ptr<const TR::RegularContent> content, OnActionClick onActionClick):
    m_content(content),
    m_onActionClick(onActionClick),
    m_mode(TREE_MODE)
{
    _ASSERT(content);
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    init();
}

void RegularContentView::setMode(Mode mode)
{
    m_mode = mode;
    init();
}

void RegularContentView::connectActionClick(OnActionClick onActionClick)
{
    m_onActionClick = onActionClick;
    init();
}

void RegularContentView::update(std::shared_ptr<const TR::Content> content)
{
    _ASSERT(content);
    m_content = std::static_pointer_cast<const TR::RegularContent>(content);
    init();
}

std::unique_ptr<Wt::WToolBar> RegularContentView::getViewBar()
{
    auto toolbar = std::make_unique<Wt::WToolBar>();    

    auto treeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Tree");
    treeBtn->setCheckable(true);
    treeBtn->setChecked(m_mode == Mode::TREE_MODE);

    auto tableBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Table");
    tableBtn->setCheckable(true);
    tableBtn->setChecked(m_mode == Mode::TABLE_MODE);
    
    treeBtn->clicked().connect([tableBtn, treeBtn, this](Wt::WMouseEvent) {
        //m_treeBtn->setChecked(true);
        tableBtn->setChecked(false);
        treeBtn->setChecked(true);
        setMode(Mode::TREE_MODE);   
    }); 

    tableBtn->clicked().connect([tableBtn, treeBtn, this](Wt::WMouseEvent) {
        //m_tableBtn->setChecked(true);
        treeBtn->setChecked(false);
        tableBtn->setChecked(true);
        setMode(Mode::TABLE_MODE);      
    });

    return toolbar;
}

void RegularContentView::init()
{
    auto document = std::shared_ptr<const TR::XML::XmlRegularDoc>(m_content, &m_content->as_regular_doc());
    switch (m_mode)
    {
    case Mode::TREE_MODE:
    {
        auto model = std::make_unique<RegularDocModel>(const_cast<TR::XmlRegularDoc&>(m_content->as_regular_doc()));
        model->addColumn(std::make_unique<RegularNameColumn>());
        model->addColumn(std::make_unique<RegularValueColumn>());
        model->addColumn(std::make_unique<RegularDescriptionColumn>());

        auto treeView = std::make_unique<RegularDocTreeView>(std::move(model));
        treeView->setColumnWidth(0, 250);
        treeView->setColumnWidth(1, 200);
        treeView->setColumnWidth(2, 250);

        if (m_onActionClick)
        {
            auto model = treeView->getRegularDocModel();
            model->addColumn(std::make_unique<RegularActionColumn>(), 2);
            treeView->setColumnWidth(2, 200);
            treeView->connectActionClick(m_onActionClick);
        }

        setCurrentView(std::move(treeView));
        break;
    }

    case Mode::TABLE_MODE:
        setCurrentView(std::make_unique<RegularDocTableView>(document, m_content->get_def_doc()));
        break;

    default:
        _ASSERT(false);
    }
}

Wt::WWidget* RegularContentView::getCurrentView()
{
    auto vbox = layout();
    _ASSERT(vbox->count() == 1);
    return vbox->itemAt(0)->widget();
}

Wt::WWidget* RegularContentView::setCurrentView(std::unique_ptr<Wt::WWidget> currentView)
{       
    auto vbox = Wt::clear(this);
    auto pointer = currentView.get();
    vbox->addWidget(std::move(currentView));
    return pointer;
}