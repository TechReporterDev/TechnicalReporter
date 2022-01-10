#include "stdafx.h"
#include "WelcomePanels.h"
#include "AlertsTable.h"
#include "ChangesTable.h"
#include "Application.h"

class StatsTable: public Wt::WContainerWidget
{
public:
    StatsTable()
    {
        setPadding(5);
        setWidth(200);

        //setOverflow(Wt::WContainerWidget::Overflow::OverflowAuto);
    }

    void addStat(const Wt::WString& name, const Wt::WString& value)
    {
        auto container = addWidget(std::make_unique<Wt::WContainerWidget>());
        auto hbox = container->setLayout(std::make_unique<Wt::WHBoxLayout>());
        hbox->setContentsMargins(3, 3, 3, 3);
        hbox->addWidget(std::make_unique<Wt::WText>(name), 1);
        hbox->addWidget(std::make_unique<Wt::WText>(value));
    }
};

class StatsPalette: public Wt::Chart::WStandardPalette
{
public:
    StatsPalette(): 
        Wt::Chart::WStandardPalette(Wt::Chart::PaletteFlavour::Neutral)
    {
    }

    virtual Wt::WColor color(int index) const override
    {
        switch (index)
        {
        case 0:
            return Wt::WColor(0x8a, 0xbe, 0x4f);        

        case 1:
            return Wt::WColor(0xf6, 0x8f, 0x1e);

        case 2:
            return Wt::WColor(0xf5, 0x49, 0x3d);                                    

        default:
            _ASSERT(false);         
        }
        return Wt::WColor(0xFF, 0xFF, 0xFF); //to supress warnings
    }

    virtual Wt::WColor fontColor(int index) const override
    {
        return Wt::WColor(0, 0, 0);
    }
};

class StatsPie: public Wt::WContainerWidget
{
public:
    StatsPie():
        m_model(std::make_shared<Wt::WStandardItemModel>()),
        m_chart(nullptr)
    {
        setPadding(5);
        setContentAlignment(Wt::AlignmentFlag::Center);
        //setOverflow(Wt::WContainerWidget::Overflow::OverflowHidden);      

        m_model->insertColumns(m_model->columnCount(), 2);
        m_model->setHeaderData(0, Wt::WString(L"Name"));
        m_model->setHeaderData(1, Wt::WString(L"Value"));

        //auto caption = new Wt::WText(L"Sources state:", this);
        m_chart = addWidget(std::make_unique<Wt::Chart::WPieChart>());
        m_chart->setModel(m_model);
        m_chart->setLabelsColumn(0);
        m_chart->setDataColumn(1);

        m_chart->setMargin(10, Wt::Side::Top | Wt::Side::Bottom);
        
        m_chart->setPalette(std::make_unique<StatsPalette>());
        m_chart->setDisplayLabels(Wt::Chart::LabelOption::Inside | Wt::Chart::LabelOption::TextPercentage);
        m_chart->setStartAngle(0);
        //chart->setPerspectiveEnabled(true, 0);
        //chart->setShadowEnabled(true);
        m_chart->resize(200, 170);
    }

    void reset()
    {
        m_model->removeRows(0, m_model->rowCount());        
    }

    void addStat(const Wt::WString& name, int value)
    {
        if (value == 0)
        {
            return;
        }

        int position = m_model->rowCount();
        m_model->insertRows(position, 1);
        m_model->setData(position, 0, Wt::WString(name));
        m_model->setData(position, 1, value);
        m_chart->setExplode(position, 0.06);        
    }

private:
    std::shared_ptr<Wt::WStandardItemModel> m_model;
    Wt::Chart::WPieChart* m_chart;
};

SummaryPanel::SummaryPanel():
    Panel("Summary"),
    m_statsTable(nullptr),
    m_statsPie(nullptr),
    m_dirty(false)
{
    m_body->setOverflow(Wt::Overflow::Auto);
        
    auto container = m_body->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto hbox = container->setLayout(std::make_unique<Wt::WHBoxLayout>());
    hbox->setContentsMargins(9, 0, 9, 9);
    hbox->setSpacing(10);

    m_statsPie = hbox->addWidget(std::make_unique<StatsPie>());
    m_statsTable = hbox->addWidget(std::make_unique<StatsTable>());
    hbox->addStretch(1);

    updateStats();  
}

void SummaryPanel::updateStats()
{
    auto summaryInfo = m_client->getSummaryInfo();

    m_statsTable->clear();
    m_statsTable->addStat("Sources count:", std::to_string(summaryInfo.m_sources_count));
    m_statsTable->addStat("OK sources:", std::to_string(summaryInfo.m_sources_count - summaryInfo.m_warning_sources - summaryInfo.m_failed_sources));
    m_statsTable->addStat("Warning sources:", std::to_string(summaryInfo.m_warning_sources));
    m_statsTable->addStat("Failed sources:", std::to_string(summaryInfo.m_failed_sources));
        
    m_statsPie->reset();
    m_statsPie->addStat("OK:", summaryInfo.m_sources_count - summaryInfo.m_warning_sources - summaryInfo.m_failed_sources);
    m_statsPie->addStat("Warnings:", summaryInfo.m_warning_sources);
    m_statsPie->addStat("Failed:", summaryInfo.m_failed_sources);

    m_dirty = false;
}

void SummaryPanel::invalidate()
{
    if (m_dirty)
    {
        return;
    }

    Wt::WTimer::singleShot(std::chrono::seconds(10), this, &SummaryPanel::updateStats);
    m_dirty = true;
}

void SummaryPanel::onAddSource(const TR::SourceInfo& sourceInfo)
{
    invalidate();
}
    
void SummaryPanel::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    invalidate();
}

void SummaryPanel::onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources)
{
    invalidate();
}

MessagePanel::MessagePanel(const SourceInfoByKey& sourcesInfo, const ReportTypeInfoByUUID& reportTypesInfo, const ActionInfoByUUID& actionsInfo, const ShortcutInfoByUUID& shortcutsInfo):
    Panel("Alerts"),
    m_sourcesInfo(sourcesInfo),
    m_reportTypesInfo(reportTypesInfo),
    m_actionsInfo(actionsInfo),
    m_shortcutsInfo(shortcutsInfo)
{
    auto vbox = getBody()->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    m_alertsTable = vbox->addWidget(std::make_unique<AlertsTable>());

    auto& ds = m_alertsTable->decorationStyle();
    ds.setBorder(Wt::WBorder());

    m_alertsTable->addColumn(std::make_unique<AlertSeverityColumn>());
    m_alertsTable->addColumn(std::make_unique<AlertSourceColumn>(m_sourcesInfo));
    m_alertsTable->addColumn(std::make_unique<AlertResourceColumn>(m_reportTypesInfo, m_actionsInfo, m_shortcutsInfo));
    m_alertsTable->addColumn(std::make_unique<AlertMessageColumn>());
        
    for (auto& sourceInfo : m_sourcesInfo | boost::adaptors::map_values)
    {
        addAlerts(sourceInfo);
    }
}

void MessagePanel::addAlerts(const TR::SourceInfo& sourceInfo)
{
    for (auto& alert : sourceInfo.m_alerts)
    {
        m_alertsTable->addRow(alert);
    }
}

void MessagePanel::removeAlerts(TR::SourceKey sourceKey)
{
    for (size_t position = 0; position < m_alertsTable->getRowCount();)
    {
        if (boost::polymorphic_get<TR::Alert>(m_alertsTable->getData(position)).m_source_key == sourceKey)
        {
            m_alertsTable->removeRow(position);
            continue;
        }
        ++position;
    }       
}

void MessagePanel::onAddSource(const TR::SourceInfo& sourceInfo)
{   
    addAlerts(sourceInfo);
}
    
void MessagePanel::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    removeAlerts(sourceInfo.m_key);
    addAlerts(sourceInfo);
}

void MessagePanel::onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources)
{
    for (auto sourceKey : removedSources)
    {
        removeAlerts(sourceKey);
    }
}

ChangesPanel::ChangesPanel(const SourceInfoByKey& sourcesInfo, const ReportTypeInfoByUUID& reportTypesInfo, const StreamTypeInfoByUUID& streamTypesInfo):
    Panel("Changes"),
    m_sourcesInfo(sourcesInfo),
    m_reportTypesInfo(reportTypesInfo),
    m_streamTypesInfo(streamTypesInfo)
{
    auto vbox = getBody()->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    m_changesTable = vbox->addWidget(std::make_unique<ChangesTable>());

    auto& ds = m_changesTable->decorationStyle();
    ds.setBorder(Wt::WBorder());

    m_changesTable->addColumn(std::make_unique<ChangeTimeColumn>());
    m_changesTable->addColumn(std::make_unique<ChangeSourceColumn>(m_sourcesInfo));
    m_changesTable->addColumn(std::make_unique<ChangeResourceColumn>(m_reportTypesInfo, m_streamTypesInfo));

    for (auto& sourceInfo : m_sourcesInfo | boost::adaptors::map_values)
    {
        addChanges(sourceInfo);
    }
}

void ChangesPanel::onAddSource(const TR::SourceInfo& sourceInfo)
{
    addChanges(sourceInfo);
}

void ChangesPanel::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    removeChanges(sourceInfo.m_key);
    addChanges(sourceInfo);
}

void ChangesPanel::onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources)
{
    for (auto sourceKey : removedSources)
    {
        removeChanges(sourceKey);
    }
}

void ChangesPanel::addChanges(const TR::SourceInfo& sourceInfo)
{
    auto from_time = time(nullptr) - ONE_DAY;
    for (auto& currentReport : sourceInfo.m_current_reports)
    {
        if (currentReport.m_modified_time > from_time)
        {
            m_changesTable->addRow(currentReport);          
        }
    }

    for (auto& streamInfo : sourceInfo.m_streams)
    {
        if (streamInfo.m_time > from_time)
        {
            m_changesTable->addRow(streamInfo);         
        }
    }
}

void ChangesPanel::removeChanges(TR::SourceKey sourceKey)
{
    for (size_t position = 0; position < m_changesTable->getRowCount();)
    {
        if (m_changesTable->getSourceKey(position) == sourceKey)
        {
            m_changesTable->removeRow(position);
            continue;
        }
        ++position;
    }
}