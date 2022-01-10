#include "stdafx.h"
#include "StreamPreviewPage.h"
#include "StreamView.h"
#include "Application.h"

StreamPreviewWidget::StreamPreviewWidget(std::unique_ptr<LazyStreamQuery> streamQuery):
    m_streamQuery(std::move(streamQuery))
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    showTitleBar();
    showMessage(L"Processing...");
    m_streamQuery->run(
        [this](const std::vector<TR::StreamMessage>& streamMessages)
        {
            showStream(streamMessages);
        },
        [this](const std::string&)
        {
            showMessage(L"Failed to get stream.");
        }
    );
}

void StreamPreviewWidget::showTitleBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    auto closeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Close");
    closeBtn->clicked().connect(this, &StreamPreviewWidget::onCloseClick);
}

void StreamPreviewWidget::showMessage(const Wt::WString& message)
{   
    auto vbox = Wt::clear(this);
    auto container = vbox->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->addWidget(std::make_unique<Wt::WText>(message));
}

void StreamPreviewWidget::showStream(const std::vector<TR::StreamMessage>& streamMessages)
{   
    if (streamMessages.empty())
    {
        showMessage("Stream is empty");
        return;
    }

    auto vbox = Wt::clear(this);
    auto messageDef = streamMessages[0].m_body->get_def_doc();
    auto streamView = vbox->addWidget(std::make_unique<StreamView>(messageDef));

    for (auto& streamMessage : streamMessages)
    {
        streamView->addMessage(streamMessage);
    }
}

void StreamPreviewWidget::onCloseClick()
{
    Application::popPage();
}

StreamPreviewPage::StreamPreviewPage(std::unique_ptr<LazyStreamQuery> streamQuery):
    MainFramePage(L"Stream Preview", "/source/stream_preview"),
    m_widget(std::move(streamQuery))
{
}

MainFrameWidget* StreamPreviewPage::getWidget()
{
    return &m_widget;
}