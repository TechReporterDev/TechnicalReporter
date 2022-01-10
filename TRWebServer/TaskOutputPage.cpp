#include "stdafx.h"
#include "TaskOutputPage.h"
#include "ContentView.h"
#include "RegularContentView.h"
#include "ActionDlg.h"
#include "SourcePage.h"
#include "Application.h"

TaskOutputWidget::TaskOutputWidget(const TR::SourceInfo& sourceInfo, std::shared_ptr<TR::Content> content):
    m_sourceInfo(sourceInfo),
    m_content(content)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    showContent();
    showTitleBar();
}

void TaskOutputWidget::showTitleBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    if (auto contentView = getContentView())
    {
        if (auto viewBar = contentView->getViewBar())
        {
            toolbar->addWidget(std::move(viewBar));
        }
    }
}

void TaskOutputWidget::showContent()
{
    auto contentView = createContentView(m_content);    
    if (auto regularContentView = dynamic_cast<RegularContentView*>(contentView.get()))
    {
        regularContentView->connectActionClick([this](const TR::XML::XmlRegularNode* contextNode, const TR::XML::META::XmlActionNode* actionNode){
            auto actionInfo = m_client->getActionInfo(TR::ActionUUID(actionNode->get_action_uuid()));
            auto params = m_client->createParams(actionInfo.m_uuid, as_string(*actionNode->get_params(*contextNode)));
            auto launchDlg = new LaunchDlg(m_sourceInfo, actionInfo, std::move(params));
            launchDlg->show();
        });
    }

    setContentView(std::move(contentView));
}

void TaskOutputWidget::setContentView(std::unique_ptr<ContentView> contentView)
{   
    auto vbox = Wt::clear(this);
    vbox->addWidget(std::move(contentView));
}

ContentView* TaskOutputWidget::getContentView()
{
    auto vbox = layout();
    if (!vbox->count())
    {
        return nullptr;
    }
    return dynamic_cast<ContentView*>(vbox->itemAt(0)->widget());
}

TaskOutputPage::TaskOutputPage(const std::wstring& caption, const TR::SourceInfo& sourceInfo, std::shared_ptr<TR::Content> content):
    MainFramePage(caption, "/task_output"),
    m_widget(std::make_unique<TaskOutputWidget>(sourceInfo, content))
{
    m_path = SourcePage(sourceInfo).getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* TaskOutputPage::getWidget()
{
    return m_widget.get();
}