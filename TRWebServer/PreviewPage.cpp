#include "stdafx.h"
#include "PreviewPage.h"
#include "ContentView.h"
#include "Application.h"

PreviewWidget::PreviewWidget(std::unique_ptr<LazyContentQuery> previewContent):
    m_previewContent(std::move(previewContent))
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    vbox->addWidget(std::make_unique<EmptyContentView>(L"Preparing preview..."));

    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   
    auto closeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Cancel"); 
    closeBtn->clicked().connect(this, &PreviewWidget::onCloseClick);

    m_previewContent->run(
        [this](std::shared_ptr<TR::Content> content){
            onContentReady(content);
        },
        [this](const std::string&){
            onContentUnavailable();
        }
    );
}

void PreviewWidget::onContentReady(std::shared_ptr<TR::Content> content)
{   
    auto vbox = Wt::clear(this);
    auto contentView = vbox->addWidget(createContentView(content));

    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    if (auto viewBar = contentView->getViewBar())
    {
        toolbar->addWidget(std::move(viewBar));
    }

    auto closeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Close");
    closeBtn->clicked().connect(this, &PreviewWidget::onCloseClick);
}

void PreviewWidget::onContentUnavailable()
{   
    auto vbox = Wt::clear(this);
    vbox->addWidget(std::make_unique<EmptyContentView>(L"Content not available"));
}

void PreviewWidget::onCloseClick()
{
    Application::popPage();
}

PreviewPage::PreviewPage(std::unique_ptr<LazyContentQuery> previewContent):
    MainFramePage(L"Report Preview", "/content_preview"),
    m_widget(std::move(previewContent))
{
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* PreviewPage::getWidget()
{
    return &m_widget;
}