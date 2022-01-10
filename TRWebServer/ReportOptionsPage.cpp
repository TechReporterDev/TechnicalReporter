#include "stdafx.h"
#include "ReportSelectionPage.h"
#include "ReportOptionsPage.h"
#include "TabWidget.h"
#include "Application.h"
#include "SubjectReportOptionsPage.h"
#include "InventoryTable.h"

class ReportSubjectOptions: public InventoryTable
{
public:
    enum SubjectType { SOURCE_SUBJECT, ROLE_SUBJECT };

    static SubjectType getSubjectType(TR::SubjectKey subject_key)
    {
        if (boost::get<TR::SourceKey>(&subject_key))
        {
            return SOURCE_SUBJECT;
        }
        return ROLE_SUBJECT;
    }

    ReportSubjectOptions(TR::ReportTypeInfo reportTypeInfo, std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo, std::map<TR::RoleKey, TR::RoleInfo>& rolesInfo, SubjectType subjectType):
        m_reportTypeInfo(reportTypeInfo),
        m_sourcesInfo(sourcesInfo),
        m_rolesInfo(rolesInfo),
        m_subjectType(subjectType)
    {
        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::ReportTypeOptions& reportTypeOptions)
    {
        m_subjectOptions.clear();
        for (auto& subjectOptionsInfo : reportTypeOptions.m_subject_options)
        {
            if (m_subjectType == getSubjectType(subjectOptionsInfo.m_subject_key))
            {
                m_subjectOptions.push_back(subjectOptionsInfo);
            }
        }

        resize(m_subjectOptions.size());
        for (size_t i = 0; i < m_subjectOptions.size(); ++i)
        {
            setItem(i, getSubjectName(m_subjectOptions[i].m_subject_key), "Description");
        }
    }

    void onSetupClick(int index)
    {
        Application::pushPage(std::make_unique<SubjectReportOptionsPage>(m_subjectOptions[index]));
    }

private:

    Wt::WString getSubjectName(TR::SubjectKey subjectKey)
    {
        if (auto sourceKey = boost::get<TR::SourceKey>(&subjectKey))
        {
            auto& sourceInfo = m_sourcesInfo[*sourceKey];
            return sourceInfo.m_name;
        }
        auto roleKey = boost::get<TR::RoleKey>(subjectKey);
        auto& roleInfo = m_rolesInfo[roleKey];
        return roleInfo.m_name;     
    }

    TR::ReportTypeInfo m_reportTypeInfo;
    SubjectType m_subjectType;
    std::vector<TR::SubjectReportOptions> m_subjectOptions;

    std::map<TR::SourceKey, TR::SourceInfo>& m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo>& m_rolesInfo;
};

class ReportModifiedOptions: public InventoryTable
{
public:
    ReportModifiedOptions(TR::ReportTypeInfo reportTypeInfo, std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo, std::map<TR::RoleKey, TR::RoleInfo>& rolesInfo):
        m_reportTypeInfo(reportTypeInfo),
        m_sourcesInfo(sourcesInfo),
        m_rolesInfo(rolesInfo)
    {
        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::ReportTypeOptions& reportTypeOptions)
    {
        m_modifiedOptions = TR::get_modified_options(reportTypeOptions);
        resize(m_modifiedOptions.size());
        for (size_t i = 0; i < m_modifiedOptions.size(); ++i)
        {
            if (auto reportGeneratingOptions = boost::get<TR::SubjectReportGenerating>(&m_modifiedOptions[i]))
            {
                setItem(i, getSubjectName(reportGeneratingOptions->m_subject_key), "Report Generating Options");
            }
            else if (auto integrityCheckingOptions = boost::get<TR::SubjectIntegrityChecking>(&m_modifiedOptions[i]))
            {
                setItem(i, getSubjectName(integrityCheckingOptions->m_subject_key), "Integrity Checking Options");
            }
            else if (auto complianceOptions = boost::get<TR::SubjectComplianceChecking>(&m_modifiedOptions[i]))
            {
                setItem(i, getSubjectName(complianceOptions->m_subject_key), "Compliance Options");
            }
            else if (auto validationOptions = boost::get<TR::SubjectValidating>(&m_modifiedOptions[i]))
            {
                setItem(i, getSubjectName(validationOptions->m_subject_key), "Validation Options");
            }
            else if (auto archivingOptions = boost::get<TR::SubjectArchiving>(&m_modifiedOptions[i]))
            {
                setItem(i, getSubjectName(archivingOptions->m_subject_key), "Archiving Options");
            }
            else
            {
                _ASSERT(false);
            }
        }
    }

    Wt::WString getSubjectName(TR::SubjectKey subjectKey)
    {
        if (auto sourceKey = boost::get<TR::SourceKey>(&subjectKey))
        {
            return m_sourcesInfo[*sourceKey].m_name;
        }
        auto roleKey = boost::get<TR::RoleKey>(subjectKey);
        return m_rolesInfo[roleKey].m_name;
    }

private:
    void onSetupClick(int index)
    {
        auto& client = Application::instance()->getClient();
        if (auto reportGeneratingOptions = boost::get<TR::SubjectReportGenerating>(&m_modifiedOptions[index]))
        {       
            auto reportOptions = client.getOptions(reportGeneratingOptions->m_subject_key, m_reportTypeInfo.m_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if(auto integrityCheckingOptions = boost::get<TR::SubjectIntegrityChecking>(&m_modifiedOptions[index]))
        {
            auto reportOptions = client.getOptions(integrityCheckingOptions->m_subject_key, m_reportTypeInfo.m_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if (auto complianceOptions = boost::get<TR::SubjectComplianceChecking>(&m_modifiedOptions[index]))
        {
            auto reportOptions = client.getOptions(complianceOptions->m_subject_key, m_reportTypeInfo.m_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if (auto validationOptions = boost::get<TR::SubjectValidating>(&m_modifiedOptions[index]))
        {
            auto reportOptions = client.getOptions(validationOptions->m_subject_key, m_reportTypeInfo.m_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else if (auto archivingOptions = boost::get<TR::SubjectValidating>(&m_modifiedOptions[index]))
        {
            auto reportOptions = client.getOptions(archivingOptions->m_subject_key, m_reportTypeInfo.m_uuid);
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }
        else
        {
            _ASSERT(false);
        }
    }

    TR::ReportTypeInfo m_reportTypeInfo;
    TR::ModifiedOptions m_modifiedOptions;

    std::map<TR::SourceKey, TR::SourceInfo>& m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo>& m_rolesInfo;   
};

ReportOptionsWidget::ReportOptionsWidget(const TR::ReportTypeInfo& reportTypeInfo):
    m_reportTypeInfo(reportTypeInfo),
    m_sourceOptions(nullptr),
    m_roleOptions(nullptr),
    m_modifiedOptions(nullptr)
{
    createToolBar();
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    auto tab = vbox->addWidget(std::make_unique<TabWidget>(), 1);

    for (auto& sourceInfo : m_client->getSourcesInfo())
    {
        m_sourcesInfo[TR::SourceKey(sourceInfo.m_key)] = sourceInfo;
    }

    for (auto& roleInfo : m_client->getRolesInfo())
    {
        m_rolesInfo[TR::RoleKey(roleInfo.m_key)] = roleInfo;
    }

    // sources tab
    auto sourcesTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Sources");
    auto sourcesLayout = sourcesTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    sourcesLayout->setContentsMargins(0, 5, 0, 0);
    m_sourceOptions = sourcesLayout->addWidget(std::make_unique<ReportSubjectOptions>(
        m_reportTypeInfo, m_sourcesInfo, m_rolesInfo, ReportSubjectOptions::SOURCE_SUBJECT));   
    
    // roles tab
    auto rolesTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Roles");
    auto rolesLayout = rolesTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    rolesLayout->setContentsMargins(0, 5, 0, 0);
    m_roleOptions = rolesLayout->addWidget(std::make_unique<ReportSubjectOptions>(
        m_reportTypeInfo, m_sourcesInfo, m_rolesInfo, ReportSubjectOptions::ROLE_SUBJECT));

    // modified tab
    auto modifiedTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Modified");
    auto modifiedLayout = modifiedTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    modifiedLayout->setContentsMargins(0, 5, 0, 0);
    m_modifiedOptions = modifiedLayout->addWidget(std::make_unique<ReportModifiedOptions>(m_reportTypeInfo, m_sourcesInfo, m_rolesInfo));
}

void ReportOptionsWidget::createToolBar()
{
}

void ReportOptionsWidget::load()
{
    auto reportTypeOptions = m_client->getReportTypeOptions(m_reportTypeInfo.m_uuid);
    m_sourceOptions->refresh(reportTypeOptions);
    m_roleOptions->refresh(reportTypeOptions);
    m_modifiedOptions->refresh(reportTypeOptions);
}

std::unique_ptr<MainFramePage> ReportOptionsPage::createPage(const std::string& url)
{
    boost::regex url_reg("/options/reports/uuid=(\\S+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::ReportTypeUUID reportTypeUUID(stl_tools::gen_uuid(match[1].str()));
    auto& client = Application::instance()->getClient();
    return std::make_unique<ReportOptionsPage>(client.getReportTypeInfo(reportTypeUUID));
}

ReportOptionsPage::ReportOptionsPage(const TR::ReportTypeInfo& reportTypeInfo):
    MainFramePage(reportTypeInfo.m_name, "/options/reports/uuid=" + stl_tools::to_string(reportTypeInfo.m_uuid)),
    m_reportTypeInfo(reportTypeInfo)
{
    m_path = ReportSelectionPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* ReportOptionsPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new ReportOptionsWidget(m_reportTypeInfo));
    }
    return m_widget.get();
}