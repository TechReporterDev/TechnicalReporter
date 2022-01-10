#include "stdafx.h"
#include "MainFrame.h"
#include "WelcomePage.h"
#include "SourcesPage.h"
#include "OptionsPage.h"
#include "AdminPage.h"
#include "SourceSelectionPage.h"
#include "SourceOptionsPage.h"
#include "RoleSelectionPage.h"
#include "RoleOptionsPage.h"
#include "ReportSelectionPage.h"
#include "ReportOptionsPage.h"
#include "ActionSelectionPage.h"
#include "ActionOptionsPage.h"
#include "SourcePage.h"
#include "CurrentReportPage.h"
#include "ArchivedReportPage.h"
#include "QueriedReportPage.h"
#include "SearchPage.h"
#include "Application.h"

MainFrameWidget::MainFrameWidget(): 
    AppEventHandler(&Application::instance()->getClient())
{
    m_titleBarHolder = std::make_unique<Wt::WContainerWidget>();
    m_titleBarHolder->setMinimumSize(0, 30);
    m_titleBarHolder->setStyleClass("tr-title-bar");
}

std::unique_ptr<Wt::WWidget> MainFrameWidget::getTitleBar()
{
    return std::make_unique<WidgetHolder>(m_titleBarHolder.get());
}

Wt::WToolBar* MainFrameWidget::setTitleBar(std::unique_ptr<Wt::WToolBar> titleBar)
{
    m_titleBarHolder->clear();
    titleBar->setFloatSide(Wt::Side::Right);
    titleBar->setMargin(0, Wt::Side::Left);
    return m_titleBarHolder->addWidget(std::move(titleBar));
}

MainFramePage::MainFramePage(std::wstring caption, std::string url):
    m_caption(std::move(caption)),
    m_url(std::move(url)),
    m_path({Link(m_caption, m_url)})
{
}

std::wstring MainFramePage::getCaption() const
{
    return m_caption;
}

std::string MainFramePage::getUrl() const
{
    return m_url;
}

std::vector<Link> MainFramePage::getPath() const
{
    return m_path;
}

MainFrameTitleBar::MainFrameTitleBar()
{   
    auto hbox = setLayout(std::make_unique<Wt::WHBoxLayout>());
    //setMinimumSize(0, 50);
}

void MainFrameTitleBar::reset(std::wstring title, std::vector<Link> links, std::unique_ptr<Wt::WWidget> widget)
{   
    auto hbox = Wt::clear<Wt::WHBoxLayout>(this);

    auto vbox = hbox->addLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->addStretch(1);
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);   

    auto titleText = vbox->addWidget(std::make_unique<Wt::WText>(title));
    titleText->setLineHeight(20);
    auto titleStyle = titleText->decorationStyle();
    auto font = titleStyle.font();
    font.setSize(Wt::WFont::Size::Larger);
    titleStyle.setFont(font);
    titleText->setDecorationStyle(titleStyle);

    if (links.size() > 1)
    {
        auto linksBox = vbox->addWidget(std::make_unique<Wt::WContainerWidget>());
        linksBox->setWidth(300);
        linksBox->addStyleClass("tr-links-block");      

        for (auto& link : links)
        {
            auto anchor = linksBox->addWidget(std::make_unique<Wt::WAnchor>(link, link.m_name));
            anchor->setPadding(0);
            anchor->setMargin(0);
            anchor->addStyleClass("tr-link");

            auto splitter = linksBox->addWidget(std::make_unique<Wt::WText>("/"));
            splitter->addStyleClass("tr-link");
        }
    }
    vbox->addStretch(1);

    if (widget)
    {
        hbox->addWidget(std::move(widget), 1);
    }   
}

MainFrameHeader::MainFrameHeader()
{
    setResponsive(true);
    setMargin(1, Wt::Side::Bottom); 

    // setup left menu
    m_leftMenu = addMenu(std::make_unique<Wt::WMenu>());
    auto itemWelcome = m_leftMenu->addItem("Welcome");
    itemWelcome->clicked().connect(std::bind([this](){
        if (auto func = m_onMarkClickHandlers[MARK_WELCOME])
        {
            func();
        }
    }));

    auto itemSources = m_leftMenu->addItem("Sources");
    itemSources->clicked().connect(std::bind([this](){
        if (auto func = m_onMarkClickHandlers[MARK_SOURCES])
        {
            func();
        }
    }));

    auto itemOptions = m_leftMenu->addItem("Options");
    itemOptions->clicked().connect(std::bind([this](){
        if (auto func = m_onMarkClickHandlers[MARK_OPTIONS])
        {
            func();
        }
    }));

    auto itemAdmin = m_leftMenu->addItem("Administrator");
    itemAdmin->clicked().connect(std::bind([this](){
        if (auto func = m_onMarkClickHandlers[MARK_ADMIN])
        {
            func();
        }
    }));
    
    // setup right menu
    auto rightMenu = addMenu(std::make_unique<Wt::WMenu>(), Wt::AlignmentFlag::Right);
    auto helpItem = rightMenu->addItem("Help"); 

    auto helpPopup = std::make_unique<Wt::WPopupMenu>();
    helpPopup->addItem("Contents");
    helpPopup->addItem("Index");
    helpPopup->addSeparator();
    helpPopup->addItem("About");

    helpPopup->itemSelected().connect(std::bind([=](Wt::WMenuItem *item) {
        Wt::WMessageBox *messageBox = new Wt::WMessageBox
            ("Help",
            Wt::WString::fromUTF8("<p>Showing Help: {1}</p>").arg(item->text()),
            Wt::Icon::Information, Wt::StandardButton::Ok);

        messageBox->buttonClicked().connect(std::bind([=]() {
            delete messageBox;
        }));

        messageBox->show();
    }, std::placeholders::_1));

    helpItem->setMenu(std::move(helpPopup));
    

    // setup search control.
    auto edit = std::make_unique<Wt::WLineEdit>();
    edit->setPlaceholderText("Enter a search");
    edit->setWidth(120);
    
    edit->focussed().connect([pedit = edit.get()](Wt::NoClass){
        pedit->setWidth(220);
    });

    edit->blurred().connect([pedit = edit.get()](Wt::NoClass){
        pedit->setWidth(120);
    });
    
    edit->enterPressed().connect(std::bind([pedit = edit.get(), this]() {
        _ASSERT(m_onSearch);
        m_onSearch(pedit->text());

        //searchResult->setText(Wt::WString("Nothing found for {1}.")
        //  .arg(edit->text()));
    }));

    addSearch(std::move(edit), Wt::AlignmentFlag::Right);
}

void MainFrameHeader::connectMarkClick(Mark mark, std::function<void(void)> onMarkClick)
{
    m_onMarkClickHandlers[mark] = onMarkClick;
}

void MainFrameHeader::connectSearch(std::function<void(std::wstring)> onSearch)
{
    _ASSERT(!m_onSearch);
    m_onSearch = onSearch;
}

void MainFrameHeader::selectMark(Mark mark)
{
    m_leftMenu->select(mark);
}

MainFrameBody::MainFrameBody()
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
}

void MainFrameBody::showWidget(Wt::WWidget* bodyWidget)
{   
    auto vbox = Wt::clear(this);
    vbox->addWidget(std::make_unique<WidgetHolder>(bodyWidget));
}

MainFrame::MainFrame():
    m_header(nullptr),
    m_titleBar(nullptr),
    m_body(nullptr)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    
    m_header = vbox->addWidget(std::make_unique<MainFrameHeader>(), 0);

    m_header->connectMarkClick(MainFrameHeader::MARK_WELCOME, [this]{
        showPage(std::make_unique<WelcomePage>());
    });

    m_header->connectMarkClick(MainFrameHeader::MARK_SOURCES, [this]{
        showPage(std::make_unique<SourcesPage>());
    });

    m_header->connectMarkClick(MainFrameHeader::MARK_OPTIONS, [this]{
        showPage(std::make_unique<OptionsPage>());
    });

    m_header->connectMarkClick(MainFrameHeader::MARK_ADMIN, [this]{
        showPage(std::make_unique<AdminPage>());
    });

    m_header->connectSearch([this](std::wstring searchString){
        showPage(std::make_unique<SearchPage>(std::move(searchString)));
    });

    m_titleBar = vbox->addWidget(std::make_unique<MainFrameTitleBar>());
    m_body = vbox->addWidget(std::make_unique<MainFrameBody>(), 1); 

    showPage(std::make_unique<WelcomePage>());  
    vbox->addWidget(std::make_unique<Wt::WText>("Technical reporter v0.2"), 0);
}

MainFrame::~MainFrame()
{
    clear();
}

void MainFrame::showUrl(const std::string& url)
{
    static std::unique_ptr<MainFramePage>(*pageFactories[])(const std::string& url) = {     
        &WelcomePage::createPage,
        &SourcesPage::createPage,
        &OptionsPage::createPage,
        &SourceSelectionPage::createPage,
        &SourceOptionsPage::createPage,
        &RoleSelectionPage::createPage,
        &RoleOptionsPage::createPage,
        &ReportSelectionPage::createPage,
        &ReportOptionsPage::createPage,
        &ActionSelectionPage::createPage,
        &ActionOptionsPage::createPage,
        &SourcePage::createPage,
        &CurrentReportPage::createPage,
        &ArchivedReportPage::createPage,
        &QueriedReportPage::createPage};

    if (!m_pageStack.empty())
    {
        if (m_pageStack.top()->getUrl() == url)
        {
            popPage();
            return;
        }
    }

    for (auto pageFactory : pageFactories)
    {
        if (auto page = pageFactory(url))
        {
            showPage(std::move(page));
        }
    }

    //show blank page
}

void MainFrame::showPage(std::unique_ptr<MainFramePage> page)
{
    setCurrentPage(std::move(page));

    while (!m_pageStack.empty())
        m_pageStack.pop();
}

void MainFrame::pushPage(std::unique_ptr<MainFramePage> page)
{
    _ASSERT(m_currentPage);
    auto currentPage = m_currentPage;   
    setCurrentPage(std::move(page));
    m_pageStack.push(currentPage);
}
void MainFrame::popPage()
{
    _ASSERT(!m_pageStack.empty());
    setCurrentPage(m_pageStack.top());
    m_pageStack.pop();
}

void MainFrame::setCurrentPage(std::shared_ptr<MainFramePage> page)
{
    auto widget = page->getWidget();
    auto titleBar = widget->getTitleBar();  

    Application::instance()->setInternalPath(page->getUrl());
    m_titleBar->reset(page->getCaption(), page->getPath(), std::move(titleBar));
    m_body->showWidget(widget);
    m_currentPage = page;
}