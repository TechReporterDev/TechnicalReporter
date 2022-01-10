#pragma once

class WidgetHolder: public Wt::WContainerWidget
{
public:
    WidgetHolder(Wt::WWidget* widget):
        m_widget(widget),
        m_vbox(nullptr)
    {
        _ASSERT(!m_widget->parent());
        m_vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
        m_vbox->setContentsMargins(0, 0, 0, 0);
        m_vbox->setSpacing(0);
        m_vbox->addWidget(std::unique_ptr<Wt::WWidget>(widget));
    }

    ~WidgetHolder()
    {
        if (m_widget)
        {
            auto widget = m_vbox->removeWidget(m_widget);
            widget.release();
            m_widget = nullptr;
        }
    }

    virtual void clear() override
    {
        if (m_vbox && m_widget)
        {
            auto widget = m_vbox->removeWidget(m_widget);
            widget.release();
            m_widget = nullptr;
        }
    }

private:    
    Wt::WWidget* m_widget;
    Wt::WVBoxLayout* m_vbox;
};