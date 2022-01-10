#include "stdafx.h"
#include "StreamSelectionPage.h"
#include "StreamOptionsPage.h"
#include "TabWidget.h"
#include "SubjectStreamOptionsPage.h"
#include "InventoryTable.h"
#include "Application.h"

class StreamSubjectOptions: public InventoryTable
{
public:
    using Predicate = std::function<bool(const TR::SubjectStreamOptions& subjectOptions)>;

    StreamSubjectOptions(TR::StreamTypeInfo streamTypeInfo, std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo, std::map<TR::RoleKey, TR::RoleInfo>& rolesInfo, Predicate predicate):
        m_streamTypeInfo(std::move(streamTypeInfo)),
        m_sourcesInfo(sourcesInfo),
        m_rolesInfo(rolesInfo),
        m_predicate(predicate)
    {
        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::StreamTypeOptions& streamTypeOptions)
    {
        m_subjectOptions.clear();
        boost::algorithm::copy_if(streamTypeOptions.m_subject_options, std::back_inserter(m_subjectOptions), m_predicate);
        
        resize(m_subjectOptions.size());
        for (size_t i = 0; i < m_subjectOptions.size(); ++i)
        {
            setItem(i, getSubjectName(m_subjectOptions[i].m_subject_key), "Description");
        }
    }

    void onSetupClick(int index)
    {
        Application::pushPage(std::make_unique<SubjectStreamOptionsPage>(m_subjectOptions[index]));
    }

protected:
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
    
    TR::StreamTypeInfo m_streamTypeInfo;    
    std::map<TR::SourceKey, TR::SourceInfo>& m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo>& m_rolesInfo;
    std::vector<TR::SubjectStreamOptions> m_subjectOptions;
    Predicate m_predicate;
};

std::unique_ptr<StreamSubjectOptions> createStreamRoleOptions(TR::StreamTypeInfo streamTypeInfo, std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo, std::map<TR::RoleKey, TR::RoleInfo>& rolesInfo)
{
    return std::make_unique<StreamSubjectOptions>(std::move(streamTypeInfo), sourcesInfo, rolesInfo, [](const TR::SubjectStreamOptions& subjectOptions){
        return subjectOptions.m_subject_key.type() == typeid(TR::RoleKey);
    });
}

std::unique_ptr<StreamSubjectOptions> createStreamSourceOptions(TR::StreamTypeInfo streamTypeInfo, std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo, std::map<TR::RoleKey, TR::RoleInfo>& rolesInfo)
{
    return std::make_unique<StreamSubjectOptions>(std::move(streamTypeInfo), sourcesInfo, rolesInfo, [](const TR::SubjectStreamOptions& subjectOptions){
        return subjectOptions.m_subject_key.type() == typeid(TR::SourceKey);
    });
}

class StreamModifiedOptions: public InventoryTable
{
public:
    StreamModifiedOptions(TR::StreamTypeInfo streamTypeInfo, std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo, std::map<TR::RoleKey, TR::RoleInfo>& rolesInfo):
        m_streamTypeInfo(streamTypeInfo),
        m_sourcesInfo(sourcesInfo),
        m_rolesInfo(rolesInfo)
    {
        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::StreamTypeOptions& streamTypeOptions)
    {
        m_modifiedOptions = TR::get_modified_options(streamTypeOptions);
        resize(m_modifiedOptions.size());
        for (size_t i = 0; i < m_modifiedOptions.size(); ++i)
        {
            if (auto streamGeneratingOptions = boost::get<TR::SubjectStreamGenerating>(&m_modifiedOptions[i]))
            {
                setItem(i, getSubjectName(streamGeneratingOptions->m_subject_key), "Stream Generating Options");
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
        if (auto streamGeneratingOptions = boost::get<TR::SubjectStreamGenerating>(&m_modifiedOptions[index]))
        {
            auto streamOptions = client.getOptions(streamGeneratingOptions->m_subject_key, m_streamTypeInfo.m_uuid);
            Application::pushPage(std::make_unique<SubjectStreamOptionsPage>(streamOptions));
        }       
        else
        {
            _ASSERT(false);
        }
    }

    TR::StreamTypeInfo m_streamTypeInfo;
    TR::ModifiedOptions m_modifiedOptions;

    std::map<TR::SourceKey, TR::SourceInfo>& m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo>& m_rolesInfo;   
};

StreamOptionsWidget::StreamOptionsWidget(const TR::StreamTypeInfo& streamTypeInfo):
    m_streamTypeInfo(streamTypeInfo),
    m_roleOptions(nullptr),
    m_sourceOptions(nullptr),
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
    m_sourceOptions = sourcesLayout->addWidget(createStreamSourceOptions(m_streamTypeInfo, m_sourcesInfo, m_rolesInfo));
    
    // roles tab
    auto rolesTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Roles");
    auto rolesLayout = rolesTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    rolesLayout->setContentsMargins(0, 5, 0, 0);
    m_roleOptions = rolesLayout->addWidget(createStreamRoleOptions(m_streamTypeInfo, m_sourcesInfo, m_rolesInfo));

    // modified tab
    auto modifiedTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Modified");
    auto modifiedLayout = modifiedTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    modifiedLayout->setContentsMargins(0, 5, 0, 0);
    m_modifiedOptions = modifiedLayout->addWidget(std::make_unique<StreamModifiedOptions>(m_streamTypeInfo, m_sourcesInfo, m_rolesInfo));
}

void StreamOptionsWidget::createToolBar()
{
}

void StreamOptionsWidget::load()
{
    auto streamTypeOptions = m_client->getStreamTypeOptions(m_streamTypeInfo.m_uuid);
    m_roleOptions->refresh(streamTypeOptions);
    m_sourceOptions->refresh(streamTypeOptions);
    m_modifiedOptions->refresh(streamTypeOptions);
}

std::unique_ptr<MainFramePage> StreamOptionsPage::createPage(const std::string& url)
{
    boost::regex url_reg("/options/streams/uuid=(\\S+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::StreamTypeUUID streamTypeUUID(stl_tools::gen_uuid(match[1].str()));
    auto& client = Application::instance()->getClient();
    return std::make_unique<StreamOptionsPage>(client.getStreamTypeInfo(streamTypeUUID));
}

StreamOptionsPage::StreamOptionsPage(const TR::StreamTypeInfo& streamTypeInfo):
    MainFramePage(streamTypeInfo.m_name, "/options/streams/uuid=" + stl_tools::to_string(streamTypeInfo.m_uuid)),
    m_streamTypeInfo(streamTypeInfo)
{
    m_path = StreamSelectionPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* StreamOptionsPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new StreamOptionsWidget(m_streamTypeInfo));
    }
    return m_widget.get();
}