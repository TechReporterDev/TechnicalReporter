#include "stdafx.h"
#include "ActionOutputPage.h"
#include "ContentView.h"
#include "RegularContentView.h"
#include "ActionDlg.h"
#include "SourcePage.h"
#include "PropertyBox.h"
#include "Application.h"

ActionOutputWidget::ActionOutputWidget(const TR::SourceInfo& sourceInfo, const TR::ActionInfo& actionInfo, TR::Params params, std::shared_ptr<TR::Content> content):
    m_sourceInfo(sourceInfo),
    m_actionInfo(actionInfo),
    m_params(std::move(params)),
    m_content(content)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    showContent();
    showTitleBar();
}

void ActionOutputWidget::showTitleBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());
    

    if (auto contentView = getContentView())
    {
        if (auto viewBar = contentView->getViewBar())
        {
            toolbar->addWidget(std::move(viewBar));
        }
    }

    auto persistBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Persist");
    persistBtn->clicked().connect(this, &ActionOutputWidget::onPersistClick);
}

void ActionOutputWidget::showContent()
{
    auto contentView = setContentView(createContentView(m_content));
    if (auto regularContentView = dynamic_cast<RegularContentView*>(contentView))
    {
        regularContentView->connectActionClick([this](const TR::XML::XmlRegularNode* contextNode, const TR::XML::META::XmlActionNode* actionNode){
            auto actionInfo = m_client->getActionInfo(TR::ActionUUID(actionNode->get_action_uuid()));
            auto params = m_client->createParams(actionInfo.m_uuid, as_string(*actionNode->get_params(*contextNode)));
            auto launchDlg = new LaunchDlg(m_sourceInfo, actionInfo, std::move(params));
            launchDlg->show();
        });
    }
}

ContentView* ActionOutputWidget::setContentView(std::unique_ptr<ContentView> contentView)
{
    auto vbox = Wt::clear(this);
    return vbox->addWidget(std::move(contentView));
}

ContentView* ActionOutputWidget::getContentView()
{
    auto vbox = layout();
    if (!vbox->count())
    {
        return nullptr;
    }
    return dynamic_cast<ContentView*>(vbox->itemAt(0)->widget());
}

void ActionOutputWidget::onPersistClick()
{
    auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Save", L"Do you want to register action shortcut,<br/>based on current parameters?", Wt::Icon::Question);
    propertyBox->addString(L"Shortcut name:", L"", L"", L"Enter shortcut name");
    propertyBox->show([this, propertyBox](){
        m_client->addCustomAction(propertyBox->getValue<std::wstring>(0), m_actionInfo.m_uuid, m_params.clone());
    });
}

ActionOutputPage::ActionOutputPage(const TR::SourceInfo& sourceInfo, const TR::ActionInfo& actionInfo, TR::Params params, std::shared_ptr<TR::Content> content):
    MainFramePage(L"Action output", "/action_output"),
    m_widget(sourceInfo, actionInfo, std::move(params), content)
{
    m_path = SourcePage(sourceInfo).getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* ActionOutputPage::getWidget()
{
    return &m_widget;
}