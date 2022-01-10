#pragma once
#include "AppClient.h"
#include "WidgetHolder.h"

struct Link: public Wt::WLink
{
    Link(const std::wstring& name, const std::string& url):
        WLink(Wt::LinkType::InternalPath, url),
        m_name(name)
    {
    }

    std::wstring m_name;
};

class MainFrameWidget: public Wt::WContainerWidget, public AppEventHandler
{
public:
    MainFrameWidget();
    std::unique_ptr<Wt::WWidget>    getTitleBar();

protected:
    Wt::WToolBar*                   setTitleBar(std::unique_ptr<Wt::WToolBar> titleBar);

private:
    std::unique_ptr<Wt::WContainerWidget> m_titleBarHolder;
};

class MainFramePage
{
public:
    MainFramePage() = default;
    MainFramePage(std::wstring caption, std::string url);
    virtual ~MainFramePage(){}

    virtual std::wstring        getCaption() const;
    virtual std::string         getUrl() const;
    virtual std::vector<Link>   getPath() const;
    virtual MainFrameWidget*    getWidget() = 0;

protected:
    std::wstring m_caption;
    std::string m_url;  
    std::vector<Link> m_path;
};

class MainFrameHeader: public Wt::WNavigationBar
{
public:
    enum Mark { MARK_WELCOME = 0, MARK_SOURCES, MARK_OPTIONS, MARK_ADMIN };

    MainFrameHeader();
    void connectMarkClick(Mark mark, std::function<void(void)> onMarkClick);
    void connectSearch(std::function<void(std::wstring)> onSearch);
    void selectMark(Mark mark);

private:
    std::map<Mark, std::function<void(void)>> m_onMarkClickHandlers;
    std::function<void(std::wstring)> m_onSearch;
    Wt::WMenu* m_leftMenu;
};

class MainFrameTitleBar: public Wt::WContainerWidget
{
public:
    MainFrameTitleBar();    
    void reset(std::wstring title, std::vector<Link> links, std::unique_ptr<Wt::WWidget> widget);
};

class MainFrameBody: public Wt::WContainerWidget
{
public:
    MainFrameBody();
    void showWidget(Wt::WWidget* bodyWidget);
};

class MainFrame: public Wt::WContainerWidget
{
public:
    MainFrame();
    ~MainFrame();

    void showUrl(const std::string& url);
    void showPage(std::unique_ptr<MainFramePage> page);
    void pushPage(std::unique_ptr<MainFramePage> page);
    void popPage();

private:
    void setCurrentPage(std::shared_ptr<MainFramePage> page);

    MainFrameHeader* m_header;
    MainFrameTitleBar* m_titleBar;
    MainFrameBody* m_body;
    std::shared_ptr<MainFramePage> m_currentPage;
    std::stack<std::shared_ptr<MainFramePage>> m_pageStack;
};