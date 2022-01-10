#include "stdafx.h"
#include "SourcePanel.h"
#include "Application.h"
#include "SourceSettingsPage.h"
#include "CurrentReportsTable.h"
#include "CurrentReportsPage.h"
#include "AlertsTable.h"

SourceInfoPanel::SourceInfoPanel(const TR::SourceInfo& sourceInfo):
    Panel("Info"),  
    m_sourceInfo(sourceInfo)
{
    addHeaderBtn(L"Edit", [this]{
        Application::pushPage(std::make_unique<SourceSettingsPage>(m_sourceInfo.m_key, m_sourceInfo));
    });

    m_body->setPadding(9);
    m_body->setOverflow(Wt::Overflow::Auto);

    showSourceInfo();
}

void SourceInfoPanel::showSourceInfo()
{
    auto& client = Application::instance()->getClient();

    m_body->clear();
    addLine(L"Name:", m_sourceInfo.m_name); 
    
    auto sourceTypeInfo = client.getSourceTypeInfo(m_sourceInfo.m_source_type_uuid);
    addLine(L"SourceType:", sourceTypeInfo.m_name);

    if (m_sourceInfo.m_parent_key)
    {
        auto parentInfo = client.getSourceInfo(m_sourceInfo.m_parent_key);
        addLine(L"Parent:", parentInfo.m_name);
    }   

    auto roleInfo = client.getRoleInfo(m_sourceInfo.m_role_key);
    addLine(L"Role:", roleInfo.m_name);
}

void SourceInfoPanel::addLine(const Wt::WString& caption, const Wt::WString& value)
{
    auto container = m_body->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto label = container->addWidget(std::make_unique<Wt::WText>(caption));
    label->setFloatSide(Wt::Side::Left);    
    label->setWidth(200);
    container->addWidget(std::make_unique<Wt::WText>(value));
}

void SourceInfoPanel::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    if (m_sourceInfo.m_key != sourceInfo.m_key)
    {
        return;
    }

    m_sourceInfo = sourceInfo;
    showSourceInfo();
}

CurrentReportPanel::CurrentReportPanel(const TR::SourceInfo& sourceInfo):
    Panel("Reports"),
    m_currentReportsTable(nullptr),
    m_sourceInfo(sourceInfo)
{
    addHeaderBtn(L"Show more", [this]{
        Application::showPage(std::make_unique<CurrentReportsPage>(m_sourceInfo));
    });

    auto vbox = m_body->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    
    m_currentReportsTable = vbox->addWidget(std::make_unique<CurrentReportsTable>());
    auto& ds = m_currentReportsTable->decorationStyle();
    ds.setBorder(Wt::WBorder());
    showCurrentReports();
}

void CurrentReportPanel::showCurrentReports()
{
    m_currentReportsTable->clear();
    for (auto& currentReportInfo : m_sourceInfo.m_current_reports)
    {
        m_currentReportsTable->addRow(currentReportInfo);
    }

    for (auto& streamInfo : m_sourceInfo.m_streams)
    {
        m_currentReportsTable->addRow(streamInfo);
    }
}

void CurrentReportPanel::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    if (m_sourceInfo.m_key != sourceInfo.m_key)
    {
        return;
    }

    m_sourceInfo = sourceInfo;
    showCurrentReports();   
}

AlertsPanel::AlertsPanel(const TR::SourceInfo& sourceInfo):
    Panel("Alerts"),
    m_alertsTable(nullptr),
    m_sourceKey(sourceInfo.m_key)
{   
    auto vbox = getBody()->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    m_alertsTable = vbox->addWidget(std::make_unique<AlertsTable>());

    auto& ds = m_alertsTable->decorationStyle();
    ds.setBorder(Wt::WBorder());

    m_alertsTable->addColumn(std::make_unique<AlertSeverityColumn>());
    m_alertsTable->addColumn(std::make_unique<AlertResourceColumn>(m_reportTypesInfo, m_actionsInfo, m_shortcutsInfo));
    m_alertsTable->addColumn(std::make_unique<AlertMessageColumn>());   

    showAlerts(sourceInfo);

    m_reportTypesInfo = stl_tools::copy_map(m_client->getReportTypesInfo(), &TR::ReportTypeInfo::m_uuid);
    for (auto& actionInfo : m_client->getActionsInfo())
    {
        m_actionsInfo[actionInfo.m_uuid] = actionInfo;
        for (auto& shortcutInfo : actionInfo.m_shortcuts)
        {
            m_shortcutsInfo[shortcutInfo.m_uuid] = shortcutInfo;
        }
    }
}

void AlertsPanel::showAlerts(const TR::SourceInfo& sourceInfo)
{
    m_alertsTable->clear();
    for (auto& alertInfo : sourceInfo.m_alerts)
    {
        m_alertsTable->addRow(alertInfo);
    }
}

void AlertsPanel::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    if (m_sourceKey == sourceInfo.m_key)
    {
        showAlerts(sourceInfo);
    }
}
