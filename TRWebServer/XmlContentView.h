#pragma once
#include "TRWebServer.h"
#include "ContentView.h"

class XmlContentView: public ContentView
{
public:
    enum Mode { TEXT_MODE, TREE_MODE };

    XmlContentView(std::shared_ptr<const TR::XmlContent> content);
    void setMode(Mode mode);

    // ContentView override
    virtual void update(std::shared_ptr<const TR::Content> content) override;
    virtual std::unique_ptr<Wt::WToolBar> getViewBar() override;

private:
    void            init();
    Wt::WWidget*    getCurrentView();
    void            setCurrentView(std::unique_ptr<Wt::WWidget> currentView);

    std::shared_ptr<const TR::XmlContent> m_content;
    Mode m_mode;
};
