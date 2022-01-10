#include "stdafx.h"
#include "SubjectOptionsPage.h"
#include "SubjectReportOptionsPage.h"
#include "SubjectStreamOptionsPage.h"
#include "SubjectActionOptionsPage.h"
#include "InventoryTable.h"
#include "Application.h"
#include "TabWidget.h"

class SubjectReportOptions: public InventoryTable
{
public:
    SubjectReportOptions()
    {
        auto& client = Application::instance()->getClient();
        for (auto& reportTypeInfo : client.getReportTypesInfo())
        {
            m_reportTypesInfo[reportTypeInfo.m_uuid] = reportTypeInfo;
        }

        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::SubjectOptions& subjectOptions)
    {
        m_reportOptions = subjectOptions.m_report_options;
        resize(m_reportOptions.size());
        for (size_t i = 0; i < m_reportOptions.size(); ++i)
        {
            auto& reportTypeInfo = m_reportTypesInfo[m_reportOptions[i].m_report_type_uuid];
            setItem(i, reportTypeInfo.m_name, "Description");
        }
    }

private:
    void onSetupClick(int index)
    {
        Application::pushPage(std::make_unique<SubjectReportOptionsPage>(m_reportOptions[index]));
    }       

    std::vector<TR::SubjectReportOptions> m_reportOptions;
    std::map<TR::ReportTypeUUID, TR::ReportTypeInfo> m_reportTypesInfo;
};

class SubjectStreamOptions: public InventoryTable
{
public:
    SubjectStreamOptions()
    {
        auto& client = Application::instance()->getClient();
        for (auto& streamTypeInfo : client.getStreamTypesInfo())
        {
            m_streamTypesInfo[streamTypeInfo.m_uuid] = streamTypeInfo;
        }

        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::SubjectOptions& subjectOptions)
    {
        m_streamOptions = subjectOptions.m_stream_options;
        resize(m_streamOptions.size());
        for (size_t i = 0; i < m_streamOptions.size(); ++i)
        {
            auto& streamTypeInfo = m_streamTypesInfo[m_streamOptions[i].m_stream_type_uuid];
            setItem(i, streamTypeInfo.m_name, "Description");
        }
    }

private:
    void onSetupClick(int index)
    {
        Application::pushPage(std::make_unique<SubjectStreamOptionsPage>(m_streamOptions[index]));
    }

    std::vector<TR::SubjectStreamOptions> m_streamOptions;
    std::map<TR::StreamTypeUUID, TR::StreamTypeInfo> m_streamTypesInfo;
};

class SubjectActionOptions: public InventoryTable
{
public:
    SubjectActionOptions()
    {
        auto& client = Application::instance()->getClient();
        for (auto& actionInfo : client.getActionsInfo())
        {
            m_actionsInfo[actionInfo.m_uuid] = actionInfo;
        }

        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::SubjectOptions& subjectOptions)
    {
        m_actionOptions = subjectOptions.m_action_options;
        resize(m_actionOptions.size());

        for (size_t i = 0; i < m_actionOptions.size(); ++i)
        {
            setItem(i, getActionName(m_actionOptions[i]), "Description");
        }
    }   

private:
    void onSetupClick(int index)
    {
        Application::pushPage(std::make_unique<SubjectActionOptionsPage>(m_actionOptions[index]));
    }

    Wt::WString getActionName(const TR::SubjectActionOptions& subjectActionOptions)
    {       
        auto& actionInfo = m_actionsInfo[subjectActionOptions.m_action_uuid];
        Wt::WString actionName = actionInfo.m_name;
        if (subjectActionOptions.m_shortcut_uuid)
        {
            auto shortcut = boost::find(actionInfo.m_shortcuts | stl_tools::members(&TR::ActionShortcutInfo::m_uuid), *subjectActionOptions.m_shortcut_uuid).base();
            if (shortcut != actionInfo.m_shortcuts.end())
            {
                actionName += L':' + shortcut->m_name;
            }
        }
        return actionName;
    }

    std::vector<TR::SubjectActionOptions> m_actionOptions;
    std::map<TR::ActionUUID, TR::ActionInfo> m_actionsInfo;
};

class SubjectModifiedOptions: public InventoryTable
{
public:
    SubjectModifiedOptions()
    {
        auto& client = Application::instance()->getClient();
        for (auto& reportTypeInfo : client.getReportTypesInfo())
        {
            m_reportTypesInfo[reportTypeInfo.m_uuid] = reportTypeInfo;
        }

        for (auto& streamTypeInfo : client.getStreamTypesInfo())
        {
            m_streamTypesInfo[streamTypeInfo.m_uuid] = streamTypeInfo;
        }

        for (auto& actionInfo : client.getActionsInfo())
        {
            m_actionsInfo[actionInfo.m_uuid] = actionInfo;
            for (auto& shortcutInfo : actionInfo.m_shortcuts)
            {
                m_shortcutsInfo[shortcutInfo.m_uuid] = shortcutInfo;
            }
        }

        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::SubjectOptions& subjectOptions)
    {
        m_modifiedOptions = TR::get_modified_options(subjectOptions);
        resize(m_modifiedOptions.size());
        for (size_t i = 0; i < m_modifiedOptions.size(); ++i)
        {
            if (auto reportGeneratingOptions = boost::get<TR::SubjectReportGenerating>(&m_modifiedOptions[i]))
            {
                setItem(i, m_reportTypesInfo[reportGeneratingOptions->m_report_type_uuid].m_name, "Report Generating Options");
            }           
            else if (auto integrityCheckingOptions = boost::get<TR::SubjectIntegrityChecking>(&m_modifiedOptions[i]))
            {
                setItem(i, m_reportTypesInfo[integrityCheckingOptions->m_report_type_uuid].m_name, "Integrity Checking Options");
            }
            else if (auto complianceOptions = boost::get<TR::SubjectComplianceChecking>(&m_modifiedOptions[i]))
            {
                setItem(i, m_reportTypesInfo[complianceOptions->m_report_type_uuid].m_name, "Compliance Options");
            }
            else if (auto validationOptions = boost::get<TR::SubjectValidating>(&m_modifiedOptions[i]))
            {
                setItem(i, m_reportTypesInfo[validationOptions->m_report_type_uuid].m_name, "Validation Options");
            }
            else if (auto archivingOptions = boost::get<TR::SubjectArchiving>(&m_modifiedOptions[i]))
            {
                setItem(i, m_reportTypesInfo[archivingOptions->m_report_type_uuid].m_name, "Archiving Options");
            }
            else if (auto streamGeneratingOptions = boost::get<TR::SubjectStreamGenerating>(&m_modifiedOptions[i]))
            {
                setItem(i, m_streamTypesInfo[streamGeneratingOptions->m_stream_type_uuid].m_name, "Stream Generating Options");
            }
            else if (auto actionOptions = boost::get<TR::SubjectActionOptions>(&m_modifiedOptions[i]))
            {
                _ASSERT(actionOptions->m_shortcut_uuid);
                auto name = Wt::WString("{1}:{2}")
                    .arg(m_actionsInfo[actionOptions->m_action_uuid].m_name)
                    .arg(m_shortcutsInfo[*actionOptions->m_shortcut_uuid].m_name);
                setItem(i, name, "Shortcut Options");
            }
            else
            {
                _ASSERT(false);
            }
        }
    }

    void onSetupClick(int index)
    {
        auto& client = Application::instance()->getClient();

        if (auto reportGeneratingOptions = boost::get<TR::SubjectReportGenerating>(&m_modifiedOptions[index]))
        {           
            auto reportOptions = client.getOptions(reportGeneratingOptions->m_subject_key, reportGeneratingOptions->m_report_type_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if (auto integrityCheckingOptions = boost::get<TR::SubjectIntegrityChecking>(&m_modifiedOptions[index]))
        {
            auto reportOptions = client.getOptions(integrityCheckingOptions->m_subject_key, integrityCheckingOptions->m_report_type_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if (auto complianceOptions = boost::get<TR::SubjectComplianceChecking>(&m_modifiedOptions[index]))
        {
            auto reportOptions = client.getOptions(complianceOptions->m_subject_key, complianceOptions->m_report_type_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if (auto validationOptions = boost::get<TR::SubjectValidating>(&m_modifiedOptions[index]))
        {
            auto reportOptions = client.getOptions(validationOptions->m_subject_key, validationOptions->m_report_type_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if (auto archivingOptions = boost::get<TR::SubjectArchiving>(&m_modifiedOptions[index]))
        {
            auto reportOptions = client.getOptions(archivingOptions->m_subject_key, archivingOptions->m_report_type_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if (auto streamGeneratingOptions = boost::get<TR::SubjectStreamGenerating>(&m_modifiedOptions[index]))
        {
            auto streamOptions = client.getOptions(streamGeneratingOptions->m_subject_key, streamGeneratingOptions->m_stream_type_uuid);
            Application::pushPage(std::make_unique<SubjectStreamOptionsPage>(streamOptions));
        }
        else if (auto actionOptions = boost::get<TR::SubjectActionOptions>(&m_modifiedOptions[index]))
        {
            Application::pushPage(std::make_unique<SubjectActionOptionsPage>(*actionOptions));
        }
        else
        {
            _ASSERT(false);
        }   
    }

private:
    TR::ModifiedOptions m_modifiedOptions;
    std::map<TR::ReportTypeUUID, TR::ReportTypeInfo> m_reportTypesInfo;
    std::map<TR::StreamTypeUUID, TR::StreamTypeInfo> m_streamTypesInfo;
    std::map<TR::ActionUUID, TR::ActionInfo> m_actionsInfo;
    std::map<TR::UUID, TR::ActionShortcutInfo> m_shortcutsInfo;
};

SubjectOptionsWidget::SubjectOptionsWidget(TR::SubjectKey subjectKey):
    m_reportOptions(nullptr),
    m_streamOptions(nullptr),
    m_actionOptions(nullptr),
    m_modifiedOptions(nullptr), 
    m_subjectKey(subjectKey)
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    auto tab = vbox->addWidget(std::make_unique<TabWidget>(), 1);   

    // reports tab
    auto reportsTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Reports");
    auto reportsLayout = reportsTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    reportsLayout->setContentsMargins(0, 5, 0, 0);
    m_reportOptions = reportsLayout->addWidget(std::make_unique<SubjectReportOptions>());

    // reports tab
    auto streamsTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Streams");
    auto streamsLayout = streamsTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    streamsLayout->setContentsMargins(0, 5, 0, 0);
    m_streamOptions = streamsLayout->addWidget(std::make_unique<SubjectStreamOptions>());   

    // actions tab
    auto actionsTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Actions");
    auto actionsLayout = actionsTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    actionsLayout->setContentsMargins(0, 5, 0, 0);
    m_actionOptions = actionsLayout->addWidget(std::make_unique<SubjectActionOptions>());

    // actions tab
    auto modifiedTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Modified");
    auto modifiedLayout = modifiedTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    modifiedLayout->setContentsMargins(0, 5, 0, 0);
    m_modifiedOptions = modifiedLayout->addWidget(std::make_unique<SubjectModifiedOptions>());
}

void SubjectOptionsWidget::createToolBar()
{
}

void SubjectOptionsWidget::load()
{
    auto subjectOptions = m_client->getSubjectOptions(m_subjectKey);

    m_reportOptions->refresh(subjectOptions);
    m_streamOptions->refresh(subjectOptions);
    m_actionOptions->refresh(subjectOptions);
    m_modifiedOptions->refresh(subjectOptions);
}

SubjectOptionsPage::SubjectOptionsPage(std::wstring caption, std::string url):
    MainFramePage(std::move(caption), std::move(url))
{
}