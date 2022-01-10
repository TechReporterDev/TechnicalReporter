#pragma once

class TabWidget: public Wt::WCompositeWidget
{
public:
    TabWidget();
    ~TabWidget();

    Wt::WWidget* addTab(std::unique_ptr<Wt::WWidget> widget, const Wt::WString& label);

    template<class Widget>
    Widget* addTab(std::unique_ptr<Widget> widget, const Wt::WString& label)
    {
        auto pointer = widget.get();
        addTab(std::unique_ptr<Wt::WWidget>(std::move(widget)), label);
        return pointer;
    }

private:
    Wt::WTabWidget* m_headerTab;
    Wt::WContainerWidget* m_body;
    std::vector<std::unique_ptr<Wt::WWidget>> m_tabWidgets;
};