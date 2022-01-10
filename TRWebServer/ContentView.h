#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class ContentView: public Wt::WContainerWidget
{
public:
    virtual void update(std::shared_ptr<const TR::Content> content)
    {
        _ASSERT(false);
    }

    virtual std::unique_ptr<Wt::WToolBar> getViewBar()
    {
        return nullptr;
    }
};

class EmptyContentView: public ContentView
{
public:
    EmptyContentView(const Wt::WString& text = L"Empty content");
    EmptyContentView(std::unique_ptr<Wt::WContainerWidget> body);

    virtual void update(std::shared_ptr<const TR::Content> content)
    {   
    }
};

std::unique_ptr<ContentView> createContentView(std::shared_ptr<const TR::Content> content);