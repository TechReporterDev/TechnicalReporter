#include "stdafx.h"
#include "StreamPage.h"
#include "StreamView.h"
#include "StreamFilterPage.h"
#include "Application.h"

StreamWidget::StreamWidget(const TR::StreamInfo& streamInfo, time_t start, time_t stop, bool autoRefresh):
    m_streamInfo(streamInfo),
    m_start(start),
    m_stop(stop),
    m_autoRefresh(autoRefresh),
    m_streamView(nullptr),
    m_messageDef(m_client->getMessageDef(m_streamInfo.m_stream_type_uuid))
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    showTitleBar();
    showMessage(L"Loading...");
    queryStream(m_start, m_stop);
}

void StreamWidget::showTitleBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    auto advancedBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Advanced");

    auto advancedMenu = std::make_unique<Wt::WPopupMenu>();
    auto filtrateItem = advancedMenu->addItem("Filtrate");
    filtrateItem->clicked().connect(this, &StreamWidget::onFiltrateClick);

    advancedBtn->setMenu(std::move(advancedMenu));
}

void StreamWidget::showMessage(const Wt::WString& message)
{   
    auto vbox = Wt::clear(this);
    auto container = vbox->addWidget(std::make_unique<Wt::WContainerWidget>()); 
    container->addWidget(std::make_unique<Wt::WText>(message));
}

void StreamWidget::showStream(const TR::StreamSegment& streamSegment)
{   
    if (!m_streamView)
    {       
        auto vbox = Wt::clear(this);
        m_streamView = vbox->addWidget(std::make_unique<StreamView>(m_messageDef));     
    }

    auto scrollPosition = m_streamView->getScrollPosition();    

    for (auto& message : streamSegment.m_messages)
    {
        m_streamView->addMessage(message);
    }

    if (scrollPosition == StreamView::BOTTOM)
    {
        m_streamView->scrollBottom();
    }
}

void StreamWidget::queryStream(time_t start, time_t stop)
{
    m_streamQuery = m_client->queryStreamSegment(m_streamInfo.m_source_key, m_streamInfo.m_stream_type_uuid, start, stop,
        [this](const TR::StreamSegment& streamSegment)
        {
            showStream(streamSegment);
            if (m_autoRefresh)
            {
                Wt::WTimer::singleShot(std::chrono::seconds(2 * DELAY), this, &StreamWidget::refresh);
            }
        },

        [this](const std::string&)
        {
            showMessage(L"Failed to load stream from server");
        }
    );
}

void StreamWidget::onFiltrateClick()
{
    std::vector<TR::StreamMessage> previewStream;
    for (size_t position = 0; position < m_streamView->getRowCount(); ++position)
    {
        previewStream.push_back(m_streamView->getData(position));
    }

    auto streamTypeInfo = m_client->getStreamTypeInfo(m_streamInfo.m_stream_type_uuid);
    Application::pushPage(std::make_unique<CreateStreamFilterPage>(streamTypeInfo, std::make_unique<StreamSegmentQuery>(
        m_streamInfo.m_source_key,
        m_streamInfo.m_stream_type_uuid,
        std::move(previewStream)
    )));
}

void StreamWidget::refresh()
{
    auto next = time(nullptr) - DELAY;
    queryStream(m_stop, next);
    m_stop = next;
}

StreamPage::StreamPage(const TR::StreamInfo& streamInfo, time_t start, time_t stop, bool autoRefresh):
    MainFramePage(L"Stream", "/source/stream/key=" + std::to_string(streamInfo.m_source_key) + "/uuid=" + stl_tools::to_string(streamInfo.m_stream_type_uuid)),
    m_widget(streamInfo, start, stop, autoRefresh)
{
}

MainFrameWidget* StreamPage::getWidget()
{
    return &m_widget;
}