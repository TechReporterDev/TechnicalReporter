#include "stdafx.h"
#include "SourceSelectionPage.h"
#include "SourceOptionsPage.h"
#include "OptionsPage.h"
#include "SourceSettingsPage.h"
#include "Application.h"

class SourceList: public Wt::WTable
{
public:
    SourceList()
    {
        addStyleClass("table form-inline table-hover");
        setWidth(Wt::WLength("100%"));
        columnAt(1)->setWidth(Wt::WLength(220));
        reset();
    }

    void reset()
    {
        auto& client = Application::instance()->getClient();
        m_sourcesInfo = client.getSourcesInfo();

        for (size_t i = 0; i < m_sourcesInfo.size(); ++i)
        {
            auto infoCell = elementAt(i, 0);
            infoCell->clear();
            
            infoCell->addWidget(std::make_unique<Wt::WText>(L"<b>" + m_sourcesInfo[i].m_name + L"</b><br/> Description"));
            if (!infoCell->clicked().isConnected())
            {
                infoCell->clicked().connect([this, i](Wt::WMouseEvent){
                    onSetupClick(i);
                });
            }

            auto actionCell = elementAt(i, 1);
            actionCell->clear();

            auto editBtn = actionCell->addWidget(std::make_unique<Wt::WPushButton>("Edit"));
            editBtn->setStyleClass(L"tr_table_button");
            editBtn->clicked().connect([this, i](Wt::WMouseEvent){
                onEditClick(i);
            });     

            auto removeBtn = actionCell->addWidget(std::make_unique<Wt::WPushButton>("Remove"));
            removeBtn->setStyleClass(L"tr_table_button");
            removeBtn->clicked().connect([this, i](Wt::WMouseEvent){
                onRemoveClick(i);
            });
            

            auto setupBtn = actionCell->addWidget(std::make_unique<Wt::WPushButton>("Setup"));
            setupBtn->setStyleClass(L"tr_table_button");
                setupBtn->clicked().connect([this, i](Wt::WMouseEvent){
                onSetupClick(i);
            });
        }

        while (rowCount() > int(m_sourcesInfo.size()))
        {
            removeRow(rowCount() - 1);
        }
    }

    void onEditClick(int index)
    {
        auto& sourceInfo = m_sourcesInfo[index];
        auto settingsPage = std::make_unique<SourceSettingsPage>(sourceInfo.m_key, sourceInfo);
        settingsPage->setOnApply([this](TR::Key sourceKey){
            reset();
        });
        Application::pushPage(std::move(settingsPage));
    }

    void onRemoveClick(int index)
    {
        auto& client = Application::instance()->getClient();
        client.removeSource(m_sourcesInfo.at(index).m_key);
        reset();
    }

    void onSetupClick(int index)
    {
        Application::showPage(std::make_unique<SourceOptionsPage>(m_sourcesInfo[index]));       
    }

private:
    std::vector<TR::SourceInfo> m_sourcesInfo;
};

SourceSelectionWidget::SourceSelectionWidget():
    m_sourceList(nullptr)
{       
    setOverflow(Wt::Overflow::Auto);
    setPadding(10);
    createToolBar();

    m_sourceList = addWidget(std::make_unique<SourceList>());   
}

void SourceSelectionWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto addBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Add");  
    addBtn->clicked().connect([this](Wt::WMouseEvent){

        TR::SourceSettings settings(
            L"",
            TR::DEFAULT_ROLE_KEY,
            TR::ROOT_GROUP_KEY,
            TR::GROUP_SOURCE_TYPE_UUID);

        auto settingsPage = std::make_unique<SourceSettingsPage>(std::move(settings));
        settingsPage->setOnApply([this](TR::Key sourceKey){
            m_sourceList->reset();
        });
        Application::pushPage(std::move(settingsPage));
    });
}

std::unique_ptr<MainFramePage> SourceSelectionPage::createPage(const std::string& url)
{
    if (url == "/options/sources")
    {
        return std::make_unique<SourceSelectionPage>();
    }
    return nullptr;
}

SourceSelectionPage::SourceSelectionPage():
    MainFramePage(L"Sources", "/options/sources")
{
    m_path = OptionsPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* SourceSelectionPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new SourceSelectionWidget());
    }
    return m_widget.get();
}