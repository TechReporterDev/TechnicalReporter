#include "stdafx.h"
#include "RoleSelectionPage.h"
#include "RoleOptionsPage.h"
#include "OptionsPage.h"
#include "RoleSettingsDlg.h"
#include "Application.h"

class RoleList: public Wt::WTable
{
public:
    RoleList()
    {
        addStyleClass("table form-inline table-hover"); 
        setWidth(Wt::WLength("100%"));
        columnAt(1)->setWidth(Wt::WLength(220));
        reset();
    }

    void reset()
    {
        auto& client = Application::instance()->getClient();
        m_rolesInfo = client.getRolesInfo();

        for (size_t i = 0; i < m_rolesInfo.size(); ++i)
        {
            auto infoCell = elementAt(i, 0);
            infoCell->clear();

            infoCell->addWidget(std::make_unique<Wt::WText>(L"<b>" + m_rolesInfo[i].m_name + L"</b><br/> Description"));
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

        while (rowCount() > int(m_rolesInfo.size()))
        {
            removeRow(rowCount() - 1);
        }
    }

    void onEditClick(int index)
    {
        auto settingsDlg = new RoleSettingsDlg(TR::RoleKey(m_rolesInfo[index].m_key));
        settingsDlg->connectApply([this](TR::RoleKey roleKey, const TR::RoleSettings& settings){
            reset();
        });

        settingsDlg->show();
    }

    void onRemoveClick(int index)
    {
        auto& client = Application::instance()->getClient();
        client.removeRole(m_rolesInfo.at(index).m_key);
        reset();
    }

    void onSetupClick(int index)
    {
        Application::showPage(std::make_unique<RoleOptionsPage>(m_rolesInfo[index]));
    }

private:
    std::vector<TR::RoleInfo> m_rolesInfo;
};

RoleSelectionWidget::RoleSelectionWidget()
{
    setPadding(10);
    createToolBar();
    setOverflow(Wt::Overflow::Auto);

    m_roleList = addWidget(std::make_unique<RoleList>());
}

void RoleSelectionWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto addBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Add");
    addBtn->clicked().connect([this](Wt::WMouseEvent){
        auto settingsDlg = new RoleSettingsDlg();
        settingsDlg->connectApply([this](TR::RoleKey roleKey, const TR::RoleSettings& settings){
            m_roleList->reset();
        });

        settingsDlg->show();
    });
}

std::unique_ptr<MainFramePage> RoleSelectionPage::createPage(const std::string& url)
{
    if (url == "/options/roles")
    {
        return std::make_unique<RoleSelectionPage>();
    }
    return nullptr;
}

RoleSelectionPage::RoleSelectionPage():
    MainFramePage(L"Roles", "/options/roles")
{
    m_path = OptionsPage().getPath();
    m_path.push_back({getCaption(), getUrl()});
}

MainFrameWidget* RoleSelectionPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new RoleSelectionWidget());
    }
    return m_widget.get();
}