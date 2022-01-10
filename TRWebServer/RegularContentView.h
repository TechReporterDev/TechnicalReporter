#pragma once
#include "TRWebServer.h"
#include "ContentView.h"

class RegularContentView: public ContentView
{
public:
    using OnActionClick = std::function<void(const TR::XML::XmlRegularNode* context_node, const TR::XML::META::XmlActionNode* action_node)>;
    enum Mode { TABLE_MODE, TREE_MODE };

    RegularContentView(std::shared_ptr<const TR::RegularContent> content, OnActionClick onActionClick = nullptr);
    void connectActionClick(OnActionClick onActionClick);
    void setMode(Mode mode);

    // ContentView override
    virtual void update(std::shared_ptr<const TR::Content> content) override;
    virtual std::unique_ptr<Wt::WToolBar> getViewBar() override;

private:
    void            init();
    Wt::WWidget*    getCurrentView();
    Wt::WWidget*    setCurrentView(std::unique_ptr<Wt::WWidget> currentView);

    std::shared_ptr<const TR::RegularContent> m_content;
    Mode m_mode;
    OnActionClick m_onActionClick;
};
