#pragma once
#include "TRWebServer.h"
#include "ContentView.h"

class ValidatedContentView: public ContentView
{
public:
    enum Mode { TABLE_MODE, TREE_MODE };

    ValidatedContentView(std::shared_ptr<const TR::RegularContent> content);
    virtual void update(std::shared_ptr<const TR::Content> content) override;
    virtual std::unique_ptr<Wt::WToolBar> getViewBar() override;
    void setMode(Mode mode);

private:
    void            init();
    Wt::WWidget*    getCurrentView();
    void            setCurrentView(std::unique_ptr<Wt::WWidget> currentView);

    std::shared_ptr<const TR::RegularContent> m_content;
    Mode m_mode;
};
