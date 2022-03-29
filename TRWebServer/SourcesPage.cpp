#include "stdafx.h"
#include "SourcesPage.h"
#include "SourcesTreeView.h"
#include "SourcesTableView.h"
#include "SourcePage.h"
#include "SourceSettingsPage.h"
#include "SourceBrowseDlg.h"
#include "SuitSettingsPage.h"
#include "RunTaskWizard.h"
#include "ContainerArray.h"
#include "PropertyBox.h"
#include "TRCore\SuitsFeature.h"
#include "Application.h"

SourcesView::Item createSourceItem(const TR::SourceInfo& sourceInfo)
{
    if (isSuitRoot(sourceInfo))
    {
        return SourcesView::SuitRootItem{sourceInfo};
    }
    else if (isSuit(sourceInfo))
    {
        return SourcesView::SuitItem{sourceInfo};
    }

    return SourcesView::SourceItem{sourceInfo};
}

SourcesWidget::SourcesWidget():
    m_sourcesView(nullptr)
{
    createToolBar();

    for (auto sourceTypeInfo : m_client->getSourceTypesInfo())
    {
        m_sourceTypes[sourceTypeInfo.m_uuid] = sourceTypeInfo;
    }

    for (auto sourceInfo : m_client->getSourcesInfo())
    {
        m_sources[sourceInfo.m_key] = sourceInfo;
    }
    
    setMode(Mode::TREE_MODE);
}

void SourcesWidget::onAddSource(const TR::SourceInfo& sourceInfo)
{
    m_sources[sourceInfo.m_key] = sourceInfo;
    m_sourcesView->addItem(createSourceItem(sourceInfo));   
}

void SourcesWidget::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    m_sources[sourceInfo.m_key] = sourceInfo;
    m_sourcesView->updateItem(createSourceItem(sourceInfo));    
}

void SourcesWidget::onRemoveSource(TR::Key, const std::vector<TR::SourceKey>& removedSources)
{
    for (auto removedSource : removedSources)
    {
        m_sources.erase(removedSource);
    }
    
    for (auto sourceKey : removedSources)
    {
        m_sourcesView->removeItem(sourceKey);
    }
}

void SourcesWidget::onAddLink(TR::Link link)
{
    m_sourcesView->addItem(SourcesView::LinkItem{link});
}

void SourcesWidget::onRemoveLink(TR::Link link)
{
    m_sourcesView->removeItem(SourcesView::LinkItem::ID(link.m_parent_key, link.m_child_key));
}

void SourcesWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());
    
    auto treeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Tree");
    treeBtn->setCheckable(true);
    treeBtn->setChecked(true);

    auto tableBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Table");  
    tableBtn->setCheckable(true);
    tableBtn->setChecked(false);

    treeBtn->clicked().connect([tableBtn, this](Wt::WMouseEvent) {
        //m_treeBtn->setChecked(true);
        tableBtn->setChecked(false);
        setMode(Mode::TREE_MODE);   
    }); 

    tableBtn->clicked().connect([treeBtn, this](Wt::WMouseEvent) {
        //m_tableBtn->setChecked(true);
        treeBtn->setChecked(false);
        setMode(Mode::TABLE_MODE);      
    });
    
    toolbar->addSeparator();

    auto addBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Add");
    addBtn->clicked().connect(std::bind(&SourcesWidget::onAddClick, this));

    auto removeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Remove");
    removeBtn->clicked().connect(std::bind(&SourcesWidget::onRemoveClick, this));

    auto editBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Edit");
    editBtn->clicked().connect(std::bind(&SourcesWidget::onEditClick, this));

    toolbar->addSeparator();
    auto runBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Run...");
    runBtn->clicked().connect(std::bind(&SourcesWidget::onRunClick, this));
}

struct OnAddClick: boost::static_visitor<void>
{
    using OnApply = std::function<void(SourcesView::ItemID addedItem)>;

    OnAddClick(OnApply onApply):
        m_onApply(onApply)
    {
    }

    void operator()(const SourcesView::SourceItem& parentItem) const
    {
        TR::SourceSettings settings{
            L"",
            TR::DEFAULT_ROLE_KEY,
            parentItem.m_sourceInfo.m_key,
            TR::GROUP_SOURCE_TYPE_UUID
        };

        auto settingsPage = std::make_unique<SourceSettingsPage>(std::move(settings));
        settingsPage->setOnApply(m_onApply);
        Application::pushPage(std::move(settingsPage));
    }

    void operator()(const SourcesView::SuitRootItem& parentItem) const
    {
        TR::SourceSettings settings(
            L"",
            TR::DEFAULT_ROLE_KEY,
            parentItem.m_sourceInfo.m_key,
            TR::Core::SuitSourceType::SOURCE_TYPE_UUID);

        auto settingsPage = std::make_unique<SuitSettingsPage>(std::move(settings));
        settingsPage->setOnApply(m_onApply);
        Application::pushPage(std::move(settingsPage));
    }

    void operator()(const SourcesView::SuitItem& parentItem) const
    {
        auto parentKey = parentItem.getID();
        auto onApply = m_onApply;
        auto browseDlg = new SourceBrowseDlg(TR::ROOT_GROUP_KEY);
        browseDlg->connectApply([onApply, parentKey](const TR::SourceInfo& childSource){
            auto& client = Application::instance()->getClient();
            client.addLink({parentKey, childSource.m_key});
            onApply(std::make_pair(parentKey, childSource.m_key));
        });
        browseDlg->show();
    }

    void operator()(const SourcesView::LinkItem& parentItem) const
    {
    }

    OnApply m_onApply;
};

void SourcesWidget::onAddClick()
{
    auto selection = m_sourcesView->getSelection();
    if (selection.size() != 1)
    {
        auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK, L"Add source", L"You should select parent source or suit", Wt::Icon::Warning);
        propertyBox->show();
        return;
    }

    boost::apply_visitor(OnAddClick([this](SourcesView::ItemID addedItem){
        Application::instance()->handleEvents();
        m_sourcesView->clearSelection();
        m_sourcesView->selectItem(addedItem);
    }), selection[0]);
}

struct OnRemoveClick: boost::static_visitor<void>
{
    OnRemoveClick():
        m_client(Application::instance()->getClient())
    {
    }

    void operator()(const SourcesView::SourceItem& sourceItem) const
    {
        m_client.removeSource(sourceItem.m_sourceInfo.m_key);
    }

    void operator()(const SourcesView::SuitRootItem& suitRootItem) const
    {
        auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK, L"Remove", L"You can not remove 'Suits'", Wt::Icon::Critical);
        propertyBox->show();
    }

    void operator()(const SourcesView::SuitItem& suitItem) const
    {
        m_client.removeSource(suitItem.m_sourceInfo.m_key);
    }

    void operator()(const SourcesView::LinkItem& linkItem) const
    {
        m_client.removeLink(linkItem.m_link);
    }

    AppClient& m_client;
};

void SourcesWidget::onRemoveClick()
{
    auto selection = m_sourcesView->getSelection();
    if (selection.size() != 1)
    {
        auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK, L"Remove source", L"You should select single source or suit to remove", Wt::Icon::Information);
        propertyBox->show();
        return;
    }

    auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Remove", L"Do you really want to remove selected source?", Wt::Icon::Question);
    propertyBox->show([this, selection]() {
        boost::apply_visitor(OnRemoveClick(), selection[0]);
    });    
}

struct OnEditClick: boost::static_visitor<void>
{
    OnEditClick():
        m_client(Application::instance()->getClient())
    {
    }

    void operator()(const SourcesView::SourceItem& sourceItem) const
    {
        Application::pushPage(std::make_unique<SourceSettingsPage>(sourceItem.getID(), sourceItem.m_sourceInfo));
    }

    void operator()(const SourcesView::SuitRootItem& suitRootItem) const
    {
        Application::pushPage(std::make_unique<SuitSettingsPage>(suitRootItem.getID(), suitRootItem.m_sourceInfo));
    }

    void operator()(const SourcesView::SuitItem& suitItem) const
    {
        Application::pushPage(std::make_unique<SuitSettingsPage>(suitItem.getID(), suitItem.m_sourceInfo));
    }

    void operator()(const SourcesView::LinkItem& linkItem) const
    {
        auto sourceInfo = m_client.getSourceInfo(linkItem.m_link.m_child_key);
        Application::pushPage(std::make_unique<SourceSettingsPage>(sourceInfo.m_key, sourceInfo));
    }   

    AppClient& m_client;
};

void SourcesWidget::onEditClick()
{
    auto selection = m_sourcesView->getSelection();
    if (selection.size() != 1)
    {
        auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK, L"Edit source", L"You should select single source or suit to edit", Wt::Icon::Information);
        propertyBox->show();
        return;
    }

    boost::apply_visitor(OnEditClick(), selection[0]);
}

void SourcesWidget::onRunClick()
{
    auto wizard = new RunTaskWizard(this);
    wizard->show();
}

struct OnShowClick: boost::static_visitor<void>
{
    OnShowClick():
        m_client(Application::instance()->getClient())
    {
    }

    void operator()(const SourcesView::SourceItem& sourceItem) const
    {
        Application::showPage(std::make_unique<SourcePage>(sourceItem.m_sourceInfo));
    }

    void operator()(const SourcesView::SuitRootItem& suitRootItem) const
    {       
    }

    void operator()(const SourcesView::SuitItem& suitItem) const
    {
        Application::showPage(std::make_unique<SourcePage>(suitItem.m_sourceInfo));
    }

    void operator()(const SourcesView::LinkItem& linkItem) const
    {
        auto sourceInfo = m_client.getSourceInfo(linkItem.m_link.m_child_key);
        Application::showPage(std::make_unique<SourcePage>(sourceInfo));
    }

    AppClient& m_client;
};

void SourcesWidget::onSourceDblClick(const SourcesView::Item& item)
{
    boost::apply_visitor(OnShowClick(), item);
}

void SourcesWidget::setMode(Mode mode)
{
    clear();
    m_sourcesView = nullptr;

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    std::vector<SourcesView::Item> items;
    for (auto& sourceInfo : m_client->getSourcesInfo())
    {
        items.push_back(createSourceItem(sourceInfo));  
    }

    for (auto& link : m_client->getLinks())
    {
        items.push_back(SourcesView::LinkItem{link});
    }

    switch (mode)
    {
    case Mode::TREE_MODE:
    {
        auto treeView = vbox->addWidget(std::make_unique<SourcesTreeView>(items, boost::bind(&SourcesWidget::onSourceDblClick, this, _1)), 1);
        treeView->addColumn(std::make_unique<SourceNameColumn>(m_sources));     
        treeView->addColumn(std::make_unique<SourceTypeColumn>(m_sourceTypes));
        treeView->expand(3);
        m_sourcesView = treeView;
        break;
    }
    case Mode::TABLE_MODE:
    {
        auto tableView = vbox->addWidget(std::make_unique<SourcesTableView>(items, boost::bind(&SourcesWidget::onSourceDblClick, this, _1)), 1);        
        tableView->addColumn(std::make_unique<SourceNameColumn>(m_sources));
        tableView->addColumn(std::make_unique<SourceTypeColumn>(m_sourceTypes));
        tableView->addColumn(std::make_unique<SourceParentColumn>(m_sources));
        m_sourcesView = tableView;
        break;
    }
    default:
        _ASSERT(false);
    }
}

std::unique_ptr<MainFramePage> SourcesPage::createPage(const std::string& url)
{
    if (url == "/sources")
    {
        return std::make_unique<SourcesPage>();
    }
    return nullptr;
}

SourcesPage::SourcesPage():
    MainFramePage(L"Sources", "/sources")
{
}

MainFrameWidget* SourcesPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new SourcesWidget());
    }
    return m_widget.get();
}