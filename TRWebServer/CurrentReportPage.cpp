#include "stdafx.h"
#include "CurrentReportPage.h"
#include "RegularReportPage.h"
#include "ComplianceReportPage.h"
#include "ValidatedReportPage.h"
#include "CompareContentPage.h"
#include "RebuildDlg.h"
#include "SourcePage.h"
#include "GetTimeDlg.h"
#include "PropertyBox.h"
#include "Application.h"

#include <Wt\Http\Request.h>
#include <Wt\Http\Response.h>
#include <Wt\WResource.h>

namespace {
class ContentResource : public Wt::WResource
{
public:
    ContentResource(Wt::WString name, std::shared_ptr<TR::TextContent> content) :
        m_content(content)
    {
        suggestFileName(name);
    }

    ~ContentResource() {
        beingDeleted();
    }

    void handleRequest(const Wt::Http::Request& request,
        Wt::Http::Response& response) {
        response.setMimeType("plain/text");
        response.out() << m_content->as_string() << std::endl;
    }
private:
    std::shared_ptr<TR::TextContent> m_content;
};
}

CurrentReportWidget::CurrentReportWidget(const TR::CurrentReportInfo& currentReportInfo):
    m_currentReportInfo(currentReportInfo)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    setCurrentReportInfo(currentReportInfo);
}

void CurrentReportWidget::showTitleBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   
        
    if (!m_currentReportInfo.m_empty)
    {
        toolbar->addWidget(std::make_unique<Wt::WText>(Application::formatTime(m_currentReportInfo.m_modified_time)));
        toolbar->addWidget(std::make_unique<Wt::WText>("(" + Application::formatTime(m_currentReportInfo.m_update_time) + ")"));
    }

    if (m_currentReportInfo.m_deferred)
    {
        toolbar->addSeparator();
        auto refreshBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Refresh");
        refreshBtn->setIcon(Wt::WLink("icons/timer-reload.png"));
        refreshBtn->setStyleClass("tr-image-button-24");
        refreshBtn->clicked().connect([&](Wt::WMouseEvent){
            queryContent();
        });     
    }

    auto contentView = getContentView();
    _ASSERT(contentView);
    if (auto viewBar = contentView->getViewBar())
    {
        toolbar->addSeparator();
        toolbar->addWidget(std::move(viewBar));
    }

    if (auto controlBar = getControlBar())
    {
        toolbar->addSeparator();
        toolbar->addWidget(std::move(controlBar));
    }
}

void CurrentReportWidget::showContent(std::shared_ptr<TR::Content> content)
{
    m_content = content;

    if (!content)
    {
        setContentView(std::make_unique<EmptyContentView>(L"Report has never been loaded from device"));
    }
    else
    {
        if (emptyContentView())
        {
            setContentView(createContentView(content));
        }
        else
        {
            getContentView()->update(content);
        }
    }
}

void CurrentReportWidget::setContentView(std::unique_ptr<ContentView> contentView)
{
    auto vbox = Wt::clear(this);    
    vbox->addWidget(std::move(contentView));
}

ContentView* CurrentReportWidget::getContentView()
{
    auto vbox = layout();
    if (!vbox->count())
    {
        return nullptr;
    }
    return dynamic_cast<ContentView*>(vbox->itemAt(0)->widget());
}

bool CurrentReportWidget::emptyContentView()
{
    return dynamic_cast<EmptyContentView*>(getContentView());
}

void CurrentReportWidget::onCompareArchivedClick()
{
    auto getTimeDlg = new GetTimeDlg("Archived time", this);
    getTimeDlg->show([this](time_t time){
        Application::showPage(std::make_unique<CompareContentPage>(
            m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid),            
            std::make_unique<ArchivedContentQuery>(m_client, m_currentReportInfo.m_source_key, m_currentReportInfo.m_report_type_uuid, time),
            std::make_unique<CurrentContentQuery>(m_client, m_currentReportInfo.m_source_key, m_currentReportInfo.m_report_type_uuid)));
    });
}

void CurrentReportWidget::onComparePatternClick()
{
    Application::pushPage(std::make_unique<CompareContentPage>(
        m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid),
        std::make_unique<PatternQuery>(m_client, m_currentReportInfo.m_source_key, m_currentReportInfo.m_report_type_uuid),
        std::make_unique<CurrentContentQuery>(m_client, m_currentReportInfo.m_source_key, m_currentReportInfo.m_report_type_uuid)));
}

void CurrentReportWidget::onSendPatternClick()
{
    auto question = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Save patters",
        L"Do you really want to replace current<br/>integrity checking pattern?", Wt::Icon::Question);
    
    question->show([this]{
        m_client->setPattern(m_currentReportInfo.m_source_key, m_currentReportInfo.m_report_type_uuid, m_content->as_blob());
        
        auto approved = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Save patters",
            L"Pattern successfully installed.", Wt::Icon::Information);
        approved->show();
    }); 
}

void CurrentReportWidget::onSendFileClick()
{
}

void CurrentReportWidget::onReloadClick()
{
    auto reportTypeInfo = m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid);
    auto sourceInfo = m_client->getSourceInfo(m_currentReportInfo.m_source_key);
    auto rebuildDlg = new RebuildDlg(sourceInfo, reportTypeInfo);
    rebuildDlg->show();
}

void CurrentReportWidget::setCurrentReportInfo(const TR::CurrentReportInfo& currentReportInfo)
{
    m_currentReportInfo = currentReportInfo;
    setContentView(std::make_unique<EmptyContentView>("Loading..."));   
    showTitleBar();
    queryContent();
}

void CurrentReportWidget::queryContent()
{
    m_contentQuery = m_client->queryCurrentContent(m_currentReportInfo.m_source_key, m_currentReportInfo.m_report_type_uuid,
        [this](std::shared_ptr<TR::Content> content){
            showContent(content);
            showTitleBar();
        },
        [this](const std::string&){
            setContentView(std::make_unique<EmptyContentView>(L"Content is not available"));
            m_content.reset();
            showTitleBar();
        }
    );
}

void CurrentReportWidget::onUpdateCurrentReport(const TR::CurrentReportInfo& currentReportInfo, bool content_changed)
{
    if (currentReportInfo.m_source_key != m_currentReportInfo.m_source_key || currentReportInfo.m_report_type_uuid != m_currentReportInfo.m_report_type_uuid)
    {       
        return;
    }

    m_currentReportInfo = currentReportInfo;
    if (content_changed)
    {
        queryContent();
    }
    else
    {
        showTitleBar();
    }
}

std::unique_ptr<Wt::WToolBar> CurrentReportWidget::getControlBar()
{
    auto toolbar = std::make_unique<Wt::WToolBar>();
    toolbar->addWidget(getAdvancedButton());
    return toolbar;
}

std::unique_ptr<Wt::WPushButton> CurrentReportWidget::getAdvancedButton()
{
    auto advancedBtn = std::make_unique<Wt::WPushButton>("Advanced");
    auto advancedMenu = std::make_unique<Wt::WPopupMenu>();
    advancedMenu->setAutoHide(true, 100);
    advancedMenu->setHideOnSelect(true);

    advancedBtn->clicked().connect([this, padvancedMenu = advancedMenu.get()](Wt::WMouseEvent){
        if (!padvancedMenu->items().empty())
        {
            return;
        }
        buildAdvancedMenu(padvancedMenu);
    });

    advancedBtn->setMenu(std::move(advancedMenu));
    return advancedBtn;
}

void CurrentReportWidget::buildAdvancedMenu(Wt::WPopupMenu *advancedMenu)
{
    if (m_content)
    {
        auto compareItem = advancedMenu->addItem("Compare to");        
        auto compareMenu = std::make_unique<Wt::WPopupMenu>();      

        compareMenu->addItem("Archived")->clicked().connect([&](Wt::WMouseEvent){
            onCompareArchivedClick();
        });

        compareMenu->addItem("Pattern")->clicked().connect([&, menu = compareMenu.get()](Wt::WMouseEvent){
            onComparePatternClick();
        });

        compareItem->setMenu(std::move(compareMenu));

        auto sendItem = advancedMenu->addItem("Send to");
        auto sendMenu = std::make_unique<Wt::WPopupMenu>();     

        sendMenu->addItem("Pattern")->clicked().connect([&](Wt::WMouseEvent){
            onSendPatternClick();
        });

        /*sendMenu->addItem("File")->clicked().connect([&](Wt::WMouseEvent) {
            onSendFileClick();
        });*/
        if (auto textContent = std::dynamic_pointer_cast<TR::TextContent>(m_content))
        {
            auto link = Wt::WLink(std::make_shared<ContentResource>(L"Content.out", textContent));
            link.setTarget(Wt::LinkTarget::Download);
            sendMenu->addItem("File")->anchor()->setLink(link);
        }

        sendItem->setMenu(std::move(sendMenu));
    }

    auto reloadItem = advancedMenu->addItem("Reload");
    reloadItem->clicked().connect([&](Wt::WMouseEvent){
        onReloadClick();
    });
}

std::unique_ptr<ContentView> CurrentReportWidget::createContentView(std::shared_ptr<TR::Content> content)
{
    return ::createContentView(content);
}

void CurrentReportWidget::updateContent(std::shared_ptr<TR::Content> content)
{
    getContentView()->update(content);
}

std::unique_ptr<MainFramePage> CurrentReportPage::createPage(const std::string& url)
{
    boost::regex url_reg("/current_report/source_key=(\\d+)/report_type_uuid=(\\S+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::SourceKey sourceKey(boost::lexical_cast<int>(match[1].str()));
    TR::ReportTypeUUID reportTypeUUID(stl_tools::gen_uuid(match[2].str()));

    auto& client = Application::instance()->getClient();
    return createCurrentReportPage(client.getCurrentReport(sourceKey, reportTypeUUID));
}

static std::string get_page_url(const TR::CurrentReportInfo& currentReportInfo)
{
    return (boost::format("/current_report/source_key=%1%/report_type_uuid=%2%")
        % currentReportInfo.m_source_key
        % stl_tools::to_string(currentReportInfo.m_report_type_uuid)).str();
}

CurrentReportPage::CurrentReportPage(const TR::CurrentReportInfo& currentReportInfo):
    MainFramePage(L"", get_page_url(currentReportInfo)),
    m_currentReportInfo(currentReportInfo)
{   
    auto& client = Application::instance()->getClient();
    auto reportTypeInfo = client.getReportTypeInfo(m_currentReportInfo.m_report_type_uuid); 
    m_caption = reportTypeInfo.m_name;

    auto sourceInfo = client.getSourceInfo(m_currentReportInfo.m_source_key);
    m_path = SourcePage(sourceInfo).getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* CurrentReportPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(createWidget());         
    }
    return m_widget.get();
}

CurrentReportWidget* CurrentReportPage::createWidget()
{
    return new CurrentReportWidget(m_currentReportInfo);
}

std::unique_ptr<CurrentReportPage> createCurrentReportPage(const TR::CurrentReportInfo& currentReportInfo)
{
    auto app = Application::instance();
    auto& client = app->getClient();
    auto reportTypeInfo = client.getReportTypeInfo(currentReportInfo.m_report_type_uuid);

    if (reportTypeInfo.m_compliance_key)
    {
        return std::make_unique<CurrentComplianceReportPage>(currentReportInfo, reportTypeInfo);
    }

    if (reportTypeInfo.m_validation_key)
    {
        return std::make_unique<CurrentValidatedReportPage>(currentReportInfo);
    }

    if (reportTypeInfo.m_syntax == TR::ReportTypeInfo::Syntax::REGULAR_XML ||       
        reportTypeInfo.m_syntax == TR::ReportTypeInfo::Syntax::COMPLIANCE_XML )
    {
        return std::make_unique<CurrentRegularReportPage>(currentReportInfo);
    }
    
    return std::make_unique<CurrentReportPage>(currentReportInfo);
}