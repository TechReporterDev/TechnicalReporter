#include "stdafx.h"
#include "FilterSetupPage.h"
#include "PreviewPage.h"
#include "NodeFiltersView.h"
#include "PropertyBox.h"
#include "PersistReportWizard.h"
#include "Application.h"

class FilterPreviewContentQuery: public LazyContentQuery
{
public:
    FilterPreviewContentQuery(TR::CustomFilterSetup filterSetup, std::shared_ptr<LazyContentQuery> inputQuery):
        LazyContentQuery(nullptr, inputQuery->getSourceKey(), inputQuery->getReportTypeUUID()),
        m_filterSetup(std::move(filterSetup)),
        m_inputQuery(inputQuery)
    {
    }

    ~FilterPreviewContentQuery()
    {
        cancel();
    }

    virtual void doRun(CompletedHandler completed_handler, FailedHandler failed_handler) override
    {
        m_inputQuery->run(
            [this, completed_handler](std::shared_ptr<TR::Content> content){
                completed_handler(m_filterSetup.apply_to(std::dynamic_pointer_cast<TR::RegularContent>(content)));
            },
            failed_handler
        );
    }

    virtual void doCancel() override
    {
        m_inputQuery->cancel();
    }

    TR::CustomFilterSetup m_filterSetup;
    std::shared_ptr<LazyContentQuery> m_inputQuery;
};

FilterSetupWidget::FilterSetupWidget()
{
}

void FilterSetupWidget::initialize()
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    auto nodeFiltersView = vbox->addWidget(std::make_unique<NodeFiltersView>(
        m_filterSetup.m_xml_node_filters, *m_filterSetup.m_filter_def, nullptr));
    
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());
    toolbar->addWidget(nodeFiltersView->getViewBar());
    toolbar->addSeparator();

    auto persistBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Persist");
    persistBtn->clicked().connect([this, nodeFiltersView](Wt::WMouseEvent){
        m_filterSetup.m_xml_node_filters = nodeFiltersView->getNodeFilters();
        onPersistClick();
    });

    if (m_previewContent)
    {
        auto previewBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Preview");
        previewBtn->clicked().connect([this, nodeFiltersView](Wt::WMouseEvent){
            m_filterSetup.m_xml_node_filters = nodeFiltersView->getNodeFilters();
            onPreviewClick();
        });
    }

    auto closeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Close");
    closeBtn->clicked().connect(this, &FilterSetupWidget::onCloseClick);
}

void FilterSetupWidget::onPreviewClick()
{
    Application::pushPage(std::make_unique<PreviewPage>(
        std::make_unique<FilterPreviewContentQuery>(m_filterSetup, m_previewContent)));
}

void FilterSetupWidget::onCloseClick()
{
    Application::popPage();
}

CreateFilterWidget::CreateFilterWidget(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::RegularContent> previewContent):
    m_inputInfo(std::move(inputInfo))
{
    m_filterSetup = m_client->createFilterSetup(inputInfo.m_uuid);
    m_previewContent = std::make_unique<CurrentContentQuery>(TR::SourceKey(), TR::ReportTypeUUID(), previewContent);
    initialize();
}

void CreateFilterWidget::onPersistClick()
{
    auto persistWizard = new PersistFilterWizard(this, m_inputInfo, m_filterSetup);
    persistWizard->show();
}

CreateFilterPage::CreateFilterPage(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::RegularContent> previewContent):
    MainFramePage(L"Create Filter", "/custom_filter"),
    m_widget(std::move(inputInfo), previewContent)
{
}

MainFrameWidget* CreateFilterPage::getWidget()
{
    return &m_widget;
}

ModifyFilterWidget::ModifyFilterWidget(TR::CustomFilterInfo filterInfo, std::shared_ptr<LazyContentQuery> previewContent):
    m_filterInfo(std::move(filterInfo))
{
    m_filterSetup = m_client->getFilterSetup(m_filterInfo.m_key);
    m_previewContent = previewContent;
    initialize();
}

void ModifyFilterWidget::onPersistClick()
{
    m_client->setFilterSetup(m_filterInfo.m_key, m_filterSetup);
}

ModifyFilterPage::ModifyFilterPage(TR::CustomFilterInfo filterInfo, std::shared_ptr<LazyContentQuery> previewContent):
    m_widget(std::move(filterInfo), previewContent)
{
}

MainFrameWidget* ModifyFilterPage::getWidget()
{
    return &m_widget;
}