#include "stdafx.h"
#include "XmlContentView.h"
#include "XmlDocTreeView.h"
#include "XmlDocModel.h"
#include "PlainTextView.h"
#include "Application.h"

XmlContentView::XmlContentView(std::shared_ptr<const TR::XmlContent> content):
    m_content(content),
    m_mode(TREE_MODE)
{
    _ASSERT(content);
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    init();
}

void XmlContentView::setMode(Mode mode)
{
    m_mode = mode;
    init();
}

void XmlContentView::update(std::shared_ptr<const TR::Content> content)
{
    _ASSERT(content);
    m_content = std::static_pointer_cast<const TR::RegularContent>(content);
    init();
}

std::unique_ptr<Wt::WToolBar> XmlContentView::getViewBar()
{
    auto toolbar = std::make_unique<Wt::WToolBar>();

    auto treeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Tree");
    treeBtn->setCheckable(true);
    treeBtn->setChecked(m_mode == Mode::TREE_MODE);

    auto textBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Text");
    textBtn->setCheckable(true);
    textBtn->setChecked(m_mode == Mode::TEXT_MODE);

    treeBtn->clicked().connect([textBtn, this](Wt::WMouseEvent) {
        //m_treeBtn->setChecked(true);
        textBtn->setChecked(false);
        setMode(Mode::TREE_MODE);   
    }); 

    textBtn->clicked().connect([treeBtn, this](Wt::WMouseEvent) {
        //m_tableBtn->setChecked(true);
        treeBtn->setChecked(false);
        setMode(Mode::TEXT_MODE);       
    });

    return toolbar;
}

void XmlContentView::init()
{
    switch (m_mode)
    {
    case Mode::TREE_MODE:
    {
        auto model = std::make_unique<XmlDocModel>(const_cast<TR::XmlDoc&>(m_content->as_xml_doc()));
        model->addColumn(std::make_unique<XmlNameColumn>());
        model->addColumn(std::make_unique<XmlTextColumn>());

        auto treeView = std::make_unique<XmlDocTreeView>(std::move(model));
        treeView->setColumnWidth(0, 250);
        treeView->setColumnWidth(1, 200);
        setCurrentView(std::move(treeView));
        break;
    }

    case Mode::TEXT_MODE:
        setCurrentView(std::make_unique<PlainTextView>(m_content->as_wstring(), m_content->get_keywords()));
        break;

    default:
        _ASSERT(false);
    }
}

Wt::WWidget* XmlContentView::getCurrentView()
{
    auto vbox = layout();
    _ASSERT(vbox->count() == 1);
    return vbox->itemAt(0)->widget();
}

void XmlContentView::setCurrentView(std::unique_ptr<Wt::WWidget> currentView)
{   
    auto vbox = Wt::clear(this);
    vbox->addWidget(std::move(currentView));
}