#include "stdafx.h"
#include "ScheduleSelectionPage.h"
#include "OptionsPage.h"
#include "ScheduleSettingsDlg.h"
#include "Application.h"

class ScheduleList: public Wt::WTable
{
public:
    ScheduleList()
    {
        addStyleClass("table form-inline table-hover"); 
        setWidth(Wt::WLength("100%"));
        columnAt(1)->setWidth(Wt::WLength(220));
        reset();
    }

    void reset()
    {
        auto& client = Application::instance()->getClient();
        m_schedulesInfo = client.getSchedulesInfo();

        for (size_t i = 0; i < m_schedulesInfo.size(); ++i)
        {
            auto infoCell = elementAt(i, 0);
            infoCell->clear();

            infoCell->addWidget(std::make_unique<Wt::WText>(L"<b>" + m_schedulesInfo[i].m_name + L"</b><br/> Description"));
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

        while (rowCount() > int(m_schedulesInfo.size()))
        {
            removeRow(rowCount() - 1);
        }
    }

    void onEditClick(int index)
    {
        auto settingsDlg = new ScheduleSettingsDlg(m_schedulesInfo.at(index));
        settingsDlg->connectApply([this]{
            reset();
        });

        settingsDlg->show();
    }

    void onRemoveClick(int index)
    {
        auto& client = Application::instance()->getClient();
        client.removeSchedule(m_schedulesInfo.at(index).m_schedule_key);
        reset();
    }

    void onSetupClick(int index)
    {
    }

private:
    std::vector<TR::ScheduleInfo> m_schedulesInfo;
};

ScheduleSelectionWidget::ScheduleSelectionWidget()
{
    setPadding(10);
    createToolBar();
    setOverflow(Wt::Overflow::Auto);

    m_scheduleList = addWidget(std::make_unique<ScheduleList>());   
}

void ScheduleSelectionWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto addBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Add");  
    addBtn->clicked().connect([this](Wt::WMouseEvent){
        auto settingsDlg = new ScheduleSettingsDlg();
        settingsDlg->connectApply([this](){
            m_scheduleList->reset();
        });

        settingsDlg->show();
    });
}

std::unique_ptr<MainFramePage> ScheduleSelectionPage::createPage(const std::string& url)
{
    if (url == "/options/schedules")
    {
        return std::make_unique<ScheduleSelectionPage>();
    }
    return nullptr;
}

ScheduleSelectionPage::ScheduleSelectionPage():
    MainFramePage(L"Schedules", "/options/roles")
{
    m_path = OptionsPage().getPath();
    m_path.push_back({getCaption(), getUrl()});
}

MainFrameWidget* ScheduleSelectionPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new ScheduleSelectionWidget());
    }
    return m_widget.get();
}