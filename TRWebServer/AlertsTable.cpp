#include "stdafx.h"
#include "AlertsTable.h"
#include "CurrentReportPage.h"
#include "PatternComparePage.h"
#include "Application.h"

void AlertSeverityColumn::create(const AlertRow& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
{
    const char* image_path = nullptr;
    const char* text = nullptr;
    switch (boost::polymorphic_get<TR::Alert>(row.getData()).m_severity)
    {
        case TR::Severity::SEVERITY_INFO:
            image_path = "icons/sun01.png";
            text = "Info";
            break;
            

        case TR::Severity::SEVERITY_WARNING:
            image_path = "icons/w_cloud.png";
            text = "Warning";
            break;

        case TR::Severity::SEVERITY_FAIL:
            image_path = "icons/snow.png";
            text = "Error";
            break;

        default:
            _ASSERT(false);
    }

    auto image = container.addWidget(std::make_unique<Wt::WImage>(Wt::WLink(image_path)));
    image->resize(16, 16);
    container.addWidget(std::make_unique<Wt::WText>(text));
}

std::wstring AlertSeverityColumn::getName() const
{
    return L"Severity";
}

std::wstring AlertSeverityColumn::getString(const AlertRow& row) const
{
    switch (boost::polymorphic_get<TR::Alert>(row.getData()).m_severity)
    {
    case TR::Severity::SEVERITY_INFO:
        return L"Info";

    case TR::Severity::SEVERITY_WARNING:
        return L"Warning";

    case TR::Severity::SEVERITY_FAIL:
        return L"Error";

    default:
        _ASSERT(false);
    }
    return L"";
}

AlertSourceColumn::AlertSourceColumn(const std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo):
    m_sourcesInfo(sourcesInfo)
{
}

std::wstring AlertSourceColumn::getName() const
{
    return L"Source";
}

std::wstring AlertSourceColumn::getString(const AlertRow& row) const
{
    auto position = m_sourcesInfo.find(boost::polymorphic_get<TR::Alert>(row.getData()).m_source_key);
    if (position == m_sourcesInfo.end())
    {
        _ASSERT(false);
        return L"Unknown";
    }
    return position->second.m_name;
}

AlertResourceColumn::AlertResourceColumn(const std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& reportTypesInfo, const std::map<TR::ActionUUID, TR::ActionInfo>& actionsInfo, const std::map<TR::UUID, TR::ActionShortcutInfo>& shortcutsInfo):
    m_reportTypesInfo(reportTypesInfo),
    m_actionsInfo(actionsInfo),
    m_shortcutsInfo(shortcutsInfo)
{
}

std::wstring AlertResourceColumn::getName() const
{
    return L"Resource";
}

std::wstring AlertResourceColumn::getString(const AlertRow& row) const
{
    auto alert = row.getData();
    if (auto checking = boost::get<TR::CheckingAlert>(&alert))
    {
        auto position = m_reportTypesInfo.find(checking->m_report_type);
        if (position != m_reportTypesInfo.end())
        {
            return position->second.m_name;
        }       
    }
    else if (auto scheduledReload = boost::get<TR::ScheduledReloadAlert>(&alert))
    {
        auto position = m_reportTypesInfo.find(scheduledReload->m_report_type);
        if (position != m_reportTypesInfo.end())
        {
            return position->second.m_name;
        }
    }
    else if (auto scheduledLaunch = boost::get<TR::ScheduledLaunchAlert>(&alert))
    {
        auto action = m_actionsInfo.find(scheduledLaunch->m_action_uuid);
        if (action != m_actionsInfo.end())
        {
            auto sortcut = m_shortcutsInfo.find(scheduledLaunch->m_shortcut_uuid);
            if (sortcut != m_shortcutsInfo.end())
            {
                return (boost::wformat(L"%1%:%2%") % action->second.m_name % sortcut->second.m_name).str();
            }
        }
    }
    else
    {
        _ASSERT(false);
    }
    _ASSERT(false);
    return L"Unknown";
}

std::wstring AlertMessageColumn::getName() const
{
    return L"Message";
}

std::wstring AlertMessageColumn::getString(const AlertRow& row) const
{
    auto alert = row.getData();
    if (auto checking = boost::get<TR::CheckingAlert>(&alert))
    {
        switch (checking->m_checking_type)
        {
        case TR::CheckingAlert::INTEGRITY_CHECKING:
            return L"Integrity checking failed";

        case TR::CheckingAlert::COMPLIANCE:
            return L"Compliance checking failed";

        case TR::CheckingAlert::VALIDATION:
            return L"Validation failed";

        default:
            _ASSERT(false);
        }       
    }
    else if (auto scheduledReload = boost::get<TR::ScheduledReloadAlert>(&alert))
    {
        return L"Sheduled reload failed";
    }
    else if (auto scheduledLaunch = boost::get<TR::ScheduledLaunchAlert>(&alert))
    {
        return L"Sheduled launch failed";
    }   
    else
    {
        _ASSERT(false);
    }
    return L"";
}

AlertsTable::AlertsTable()
{
    setSelectionMode(Wt::SelectionMode::Single);

    doubleClicked().connect(std::bind([this](Wt::WModelIndex index){
        if (!index.isValid())
        {
            return;
        }

        auto& client = Application::instance()->getClient();
        auto alert = getData(index.row());
        if (auto checking = boost::get<TR::CheckingAlert>(&alert))
        {
            if (checking->m_checking_type == TR::CheckingAlert::INTEGRITY_CHECKING)
            {
                Application::pushPage(std::make_unique<PatternComparePage>(
                    client.getSourceInfo(checking->m_source_key),
                    client.getReportTypeInfo(checking->m_report_type)
                ));
                return;
            }

            auto currentReportInfo = client.getCurrentReport(checking->m_source_key, checking->m_report_type);
            Application::pushPage(createCurrentReportPage(currentReportInfo));
        }
        else if (auto scheduledReload = boost::get<TR::ScheduledReloadAlert>(&alert))
        {
        }
        else if (auto scheduledLaunch = boost::get<TR::ScheduledLaunchAlert>(&alert))
        {
        }
        else
        {
            _ASSERT(false);
        }
    }, std::placeholders::_1));
}