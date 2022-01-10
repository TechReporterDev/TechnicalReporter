#include "stdafx.h"
#include "SearchPage.h"
#include "SubjectReportOptionsPage.h"
#include "SubjectActionOptionsPage.h"
#include "SourcePage.h"
#include "CurrentReportPage.h"
#include "Application.h"
#include "TabWidget.h"

struct SearchRecord
{
    std::wstring m_caption;
    std::wstring m_description;
    std::function<void(void)> m_clickHandler;
};

class SearchOutput: public Wt::WTable
{
public:
    SearchOutput(std::wstring searchString)
    {
        addStyleClass("table form-inline table-hover");
        setWidth(Wt::WLength("100%"));
        columnAt(1)->setWidth(Wt::WLength(220));
        boost::split(m_patterns, searchString, boost::is_any_of(L","));
    }

    void addRecord(SearchRecord searchRecord)
    {
        auto recordString = searchRecord.m_caption + L' ' + searchRecord.m_description;
        auto size = m_patterns.size();
        auto index = 0;     
        for (size_t i = 0; i < size; ++i)
        {
            boost::wregex pattern(m_patterns[i]);           
            if (boost::regex_search(recordString, pattern))
            {
                index += size - i;
            }
        }

        if (index > 0)
        {
            m_searchRecords.insert(std::make_pair(index, std::move(searchRecord)));
        }
    }

    virtual void load() override
    {
        size_t position = 0;
        for (auto& searchRecord: m_searchRecords | boost::adaptors::map_values)
        {
            auto infoCell = elementAt(position, 0);
            infoCell->clear();

            infoCell->addWidget(std::make_unique<Wt::WText>(L"<b>" + searchRecord.m_caption + L"</b><br/>" + searchRecord.m_description));
            if (!infoCell->clicked().isConnected())
            {
                infoCell->clicked().connect([&searchRecord](Wt::WMouseEvent){
                    searchRecord.m_clickHandler();
                });
            }
            ++position;
        }
    }

private:
    std::vector<std::wstring> m_patterns;
    std::multimap<int, SearchRecord, std::greater<int>> m_searchRecords;
};

SearchWidget::SearchWidget(std::wstring searchString)
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    auto container = vbox->addWidget(std::make_unique<Wt::WContainerWidget>());

    container->setOverflow(Wt::Overflow::Auto);
    auto searchOutput = container->addWidget(std::make_unique<SearchOutput>(searchString));

    for (auto& reportTypeInfo : m_client->getReportTypesInfo())
    {
        m_reportTypesInfo[reportTypeInfo.m_uuid] = reportTypeInfo;
    }

    for (auto& sourceInfo : m_client->getSourcesInfo())
    {
        m_sourcesInfo[sourceInfo.m_key] = sourceInfo;
    }

    for (auto& roleInfo : m_client->getRolesInfo())
    {
        m_rolesInfo[roleInfo.m_key] = roleInfo;
    }

    for (auto& sourceInfo : m_sourcesInfo | boost::adaptors::map_values)
    {
        searchOutput->addRecord(makeSearchRecord(sourceInfo));
        for (auto& currentReportInfo : sourceInfo.m_current_reports)
        {
            searchOutput->addRecord(makeSearchRecord(currentReportInfo));
        }

        auto sourceOptions = m_client->getSubjectOptions(sourceInfo.m_key);
        for (auto& reportOptions : sourceOptions.m_report_options)
        {
            searchOutput->addRecord(makeSearchRecord(reportOptions));
        }
    }
}

void SearchWidget::createToolBar()
{
}

SearchRecord SearchWidget::makeSearchRecord(const TR::SourceInfo& sourceInfo)
{
    return SearchRecord{sourceInfo.m_name, L"show source page", [sourceInfo]{
        Application::pushPage(std::make_unique<SourcePage>(sourceInfo));
    }};
}

SearchRecord SearchWidget::makeSearchRecord(const TR::CurrentReportInfo& currentReportInfo)
{
    return SearchRecord{m_sourcesInfo[currentReportInfo.m_source_key].m_name + L", " + m_reportTypesInfo[currentReportInfo.m_report_type_uuid].m_name,
        L"show current report", [currentReportInfo]{
        Application::pushPage(createCurrentReportPage(currentReportInfo));
    }};
}

SearchRecord SearchWidget::makeSearchRecord(const TR::SubjectReportOptions& reportOptions)
{
    if (auto sourceKey = boost::get<TR::SourceKey>(&reportOptions.m_subject_key))
    {
        return SearchRecord{m_sourcesInfo[*sourceKey].m_name + L", " + m_reportTypesInfo[reportOptions.m_report_type_uuid].m_name,
            L"show source options", [reportOptions]{
            Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
        }};
    }
    auto roleKey = boost::get<TR::RoleKey>(reportOptions.m_subject_key);
    return SearchRecord{m_rolesInfo[roleKey].m_name + L", " + m_reportTypesInfo[reportOptions.m_report_type_uuid].m_name,
        L"show role options", [reportOptions]{
        Application::pushPage(std::make_unique<SubjectReportOptionsPage>(reportOptions));
    }};
}

std::unique_ptr<MainFramePage> SearchPage::createPage(const std::string& url)
{
    boost::regex url_reg("/search");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    return std::make_unique<SearchPage>(L"");
}

SearchPage::SearchPage(std::wstring searchString):
    MainFramePage(L"Search result", "/search"),
    m_widget(searchString)
{
}

MainFrameWidget* SearchPage::getWidget()
{
    return &m_widget;
}