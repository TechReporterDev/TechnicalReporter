#include "stdafx.h"
#include "ActionSelectionPage.h"
#include "ActionOptionsPage.h"
#include "OptionsPage.h"
#include "PropertyWizard.h"
#include "CommonPropertyDoc.h"
#include "CommonPropertyPage.h"
#include "RefValue.h"
#include "ContainerArray.h"
#include "ActionDlg.h"
#include "Application.h"

class CreateShortcutWizard: public PropertyWizard
{
public:
    enum PageID { HOST_ACTION = 1, SHORTCUT_SCHEDULE, SHORTCUT_FINISH };

    CreateShortcutWizard(Wt::WContainerWidget* parent):
        PropertyWizard(L"Create shortcut"),
        m_parent(parent),
        m_client(Application::instance()->getClient()),
        m_hostAction(0),
        m_schedule(0)
    {
        m_actions = m_client.getActionsInfo();
        m_schedules = m_client.getSchedulesInfo();
        init(std::make_unique<HostActionPage>(*this));
    }

protected:
    class ShortcutParamsDlg: public PropertyDialog
    {
    public:
        ShortcutParamsDlg(CreateShortcutWizard& createShortcutWizard):
            PropertyDialog(createShortcutWizard.getHostAction().m_name),
            m_createShortcutWizard(createShortcutWizard),
            m_params(createShortcutWizard.getParams().clone())
        {
            init(std::make_unique<CommonPropertyPage>(createProperty(*m_params.m_params_doc)));
        }

    protected:
        // Dialog override
        virtual void apply()
        {
            m_createShortcutWizard.m_params = std::move(m_params);
        }

    private:
        CreateShortcutWizard& m_createShortcutWizard;
        TR::Params m_params;
    };

    class HostActionPage: public CommonPropertyPage
    {
    public:
        HostActionPage(CreateShortcutWizard& createShortcutWizard):
            CommonPropertyPage(L"Host action", L"Select host action and setup parameters for shortcut", HOST_ACTION),
            m_createShortcutWizard(createShortcutWizard)
        {
            show(CommonProperty(L"", L"",
                SelectorPropertyEx(L"", L"", L"Setup...", [](int){return 0; },
                stl_tools::copy_vector(m_createShortcutWizard.m_actions | stl_tools::members(&TR::ActionInfo::m_name)),
                customValue<int>(
                    [&]{ return m_createShortcutWizard.m_hostAction; },
                    [&](int position){ m_createShortcutWizard.setHostAction(position); }
                ))
            ));
        }

    protected:
        // CommonPropertyPage override
        virtual void handleEvent(boost::any) override
        {
            auto shortcutParamsDlg = new ShortcutParamsDlg(m_createShortcutWizard);
            shortcutParamsDlg->show();
        }

        CreateShortcutWizard& m_createShortcutWizard;
    };

    std::unique_ptr<PropertyPageWidget> createShortcutSchedulePage()
    {
        return std::make_unique<CommonPropertyPage>(CommonProperty{L"Schedule", L"Select schedule to launch shortcut.",
            SelectorProperty(L"", L"", 
                stl_tools::copy_vector(m_schedules | stl_tools::members(&TR::ScheduleInfo::m_name)),
                refValue(m_schedule))
        }, SHORTCUT_SCHEDULE);
    }

    std::unique_ptr<PropertyPageWidget> createShortcutFinishPage()
    {
        return std::make_unique<CommonPropertyPage>(CommonProperty{L"Last step", L"Enter shortcut name and click 'Finish' to persist.",
            StringProperty(L"", L"", refValue(m_name))
        }, SHORTCUT_FINISH);
    }

    const TR::Params& getParams() const
    {
        if (!m_params)
        {
            m_params = m_client.getDefaultParams(m_actions[m_hostAction].m_uuid);
        }
        return *m_params;
    }

    const TR::ActionInfo& getHostAction() const
    {
        return m_actions[m_hostAction];
    }

    void setHostAction(int position)
    {
        m_hostAction = position;
        m_params = boost::none;
    }

    // PropertyWizard override
    virtual std::unique_ptr<PropertyPageWidget> nextPage(int currentPageIdentity) override
    {
        switch (currentPageIdentity)
        {
        case HOST_ACTION:
            return createShortcutSchedulePage();

        case SHORTCUT_SCHEDULE:
            return createShortcutFinishPage();

        default:
            _ASSERT(false);
        }
        return nullptr;
    }

    // PropertyWizard override
    virtual bool isFinal(int currentPageIdentity) override
    {
        return currentPageIdentity == SHORTCUT_FINISH;
    }

    // PropertyWizard override
    virtual void onFinish() override
    {
        m_client.addCustomAction(m_name, getHostAction().m_uuid, getParams().clone());
        m_parent->refresh();
    }
    
    Wt::WContainerWidget* m_parent;
    AppClient& m_client;
    std::vector<TR::ActionInfo> m_actions;
    std::vector<TR::ScheduleInfo> m_schedules;
    int m_hostAction;
    int m_schedule;
    std::wstring m_name;
    mutable boost::optional<TR::Params> m_params;
};

ActionSelectionWidget::ActionSelectionWidget()
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);   
    m_inventory = vbox->addWidget(std::make_unique<InventoryTable>());  
    
    m_inventory->addToolEx(L"Setup", [this](int position, boost::any any){
        auto index = boost::any_cast<std::pair<int, int>>(any); 
        auto& actionInfo = m_actionsInfo[index.first];
        boost::optional<TR::UUID> shortcutUUID;
        if (index.second >= 0)
        {
            shortcutUUID = actionInfo.m_shortcuts[index.second].m_uuid;
        }
        Application::showPage(std::make_unique<ActionOptionsPage>(actionInfo, shortcutUUID));
    }, true);

    refresh();
}

void ActionSelectionWidget::refresh()
{
    m_actionsInfo = m_client->getActionsInfo(); 
    int counter = 0;
    for (int i = 0; i < int(m_actionsInfo.size()); ++i)
    {
        auto& actionInfo = m_actionsInfo[i];
        m_inventory->setItem(counter++, actionInfo.m_name, "Description", std::make_pair(i, -1));
        for (int j = 0; j < int(actionInfo.m_shortcuts.size()); ++j)
        {
            auto& shortcutInfo = actionInfo.m_shortcuts[j];
            m_inventory->setItem(counter++, actionInfo.m_name + L':' + shortcutInfo.m_name, "Description", std::make_pair(i, j));
        }
    }
    m_inventory->resize(counter);
}

void ActionSelectionWidget::load()
{
    refresh();
}

void ActionSelectionWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto createBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Create..."); 
    createBtn->clicked().connect(std::bind(&ActionSelectionWidget::onCreateClick, this));
}

void ActionSelectionWidget::onCreateClick()
{
    auto createShortcutWizard = new CreateShortcutWizard(this);
    createShortcutWizard->show();
}

std::unique_ptr<MainFramePage> ActionSelectionPage::createPage(const std::string& url)
{
    if (url == "/options/actions")
    {
        return std::make_unique<ActionSelectionPage>();
    }
    return nullptr;
}

ActionSelectionPage::ActionSelectionPage():
    MainFramePage(L"Actions", "/options/actions")
{
    m_path = OptionsPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* ActionSelectionPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new ActionSelectionWidget());
    }
    return m_widget.get();
}