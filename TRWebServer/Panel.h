#pragma once
#include "AppClient.h"

class Panel: public Wt::WContainerWidget, public AppEventHandler
{
public:
    typedef std::function<void(void)> OnClick;

    Panel(const Wt::WString& title);
    void                    addHeaderBtn(const Wt::WString& caption, OnClick onClick);
    Wt::WContainerWidget*   getBody();

protected:
    Wt::WContainerWidget* m_header;
    Wt::WContainerWidget* m_body;
};
