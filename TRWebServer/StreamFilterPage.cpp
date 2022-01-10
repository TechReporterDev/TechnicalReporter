#include "stdafx.h"
#include "StreamFilterPage.h"
#include "StreamFilterView.h"
#include "StreamPreviewPage.h"
#include "PropertyBox.h"
#include "Application.h"

class FilteredStreamQuery: public LazyStreamQuery
{
public:
    using CompletedHandler = std::function<void(const std::vector<TR::StreamMessage>&)>;
    using FailedHandler = std::function<void(const std::string&)>;

    FilteredStreamQuery(TR::StreamFilterSetup filterSetup, std::shared_ptr<LazyStreamQuery> inputQuery):
        LazyStreamQuery(inputQuery->getSourceKey(), inputQuery->getStreamTypeUUID()),
        m_filterSetup(std::move(filterSetup)),
        m_inputQuery(std::move(inputQuery))
    {
    }

    ~FilteredStreamQuery()
    {
        m_inputQuery->cancel();
    }

protected:
    virtual void doRun(CompletedHandler completedHandler, FailedHandler failedHandler) override
    {
        m_inputQuery->run(
            [this, completedHandler](std::vector<TR::StreamMessage> inputStream){
                completedHandler(m_filterSetup.apply_to(inputStream));
            },
            failedHandler
        );
    }

    TR::StreamFilterSetup m_filterSetup;
    std::shared_ptr<LazyStreamQuery> m_inputQuery;
};

StreamFilterWidget::StreamFilterWidget(std::shared_ptr<LazyStreamQuery> previewStream):
    m_previewStream(previewStream)
{
}

void StreamFilterWidget::initialize()
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    auto streamFilterView = vbox->addWidget(std::make_unique<StreamFilterView>(*m_filterSetup.m_filter_doc));

    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    auto persistBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Persist");
    persistBtn->clicked().connect([this](Wt::WMouseEvent){
        onPersistClick();
    });

    auto previewBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Preview");
    previewBtn->clicked().connect([this](Wt::WMouseEvent){
        onPreviewClick();
    });

    auto closeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Close");
    closeBtn->clicked().connect(this, &StreamFilterWidget::onCloseClick);
}

void StreamFilterWidget::onPreviewClick()
{
    Application::pushPage(std::make_unique<StreamPreviewPage>(
        std::make_unique<FilteredStreamQuery>(m_filterSetup, m_previewStream)));
}

void StreamFilterWidget::onCloseClick()
{
    Application::popPage();
}

CreateStreamFilterWidget::CreateStreamFilterWidget(TR::StreamTypeInfo inputInfo, std::shared_ptr<LazyStreamQuery> previewStream):
    StreamFilterWidget(previewStream),
    m_inputInfo(std::move(inputInfo))
{
    m_filterSetup = m_client->createFilterSetup(inputInfo.m_uuid);
    initialize();
}

void CreateStreamFilterWidget::onPersistClick()
{
    auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Save", L"<p>Do you really want to persist filter?</p>", Wt::Icon::Question);
    propertyBox->addString(L"Output name:", L"", L"", L"Enter output stream name");
    propertyBox->show([this, propertyBox](){
        m_client->addStreamFilter(propertyBox->getValue<std::wstring>(0), m_inputInfo.m_uuid, m_filterSetup);
    });
}

CreateStreamFilterPage::CreateStreamFilterPage(TR::StreamTypeInfo inputInfo, std::shared_ptr<LazyStreamQuery> previewStream):
    MainFramePage(L"Stream Filter", "/stream_filter"),
    m_widget(std::move(inputInfo), previewStream)
{
}

MainFrameWidget* CreateStreamFilterPage::getWidget()
{
    return &m_widget;
}

ModifyStreamFilterWidget::ModifyStreamFilterWidget(TR::StreamFilterInfo filterInfo, std::shared_ptr<LazyStreamQuery> previewStream):
    StreamFilterWidget(previewStream),
    m_filterInfo(std::move(filterInfo))
{
    m_filterSetup = m_client->getStreamFilterSetup(m_filterInfo.m_key);
    initialize();
}

void ModifyStreamFilterWidget::onPersistClick()
{
    m_client->setStreamFilterSetup(m_filterInfo.m_key, m_filterSetup);
}

ModifyStreamFilterPage::ModifyStreamFilterPage(TR::StreamFilterInfo filterInfo, std::shared_ptr<LazyStreamQuery> previewStream):
    m_widget(std::move(filterInfo), previewStream)
{
}

MainFrameWidget* ModifyStreamFilterPage::getWidget()
{
    return &m_widget;
}