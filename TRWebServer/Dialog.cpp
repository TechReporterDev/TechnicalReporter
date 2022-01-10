#include "stdafx.h"
#include "Dialog.h"
#include "Application.h"

Wt::WWidget* getFocus(Wt::WWidget* _parent)
{
    if (_parent->hasFocus())
    {
        return _parent;
    }

    auto parent = dynamic_cast<Wt::WWebWidget*>(_parent);
    if (!parent)
    {
        return nullptr;
    }

    for (auto childWidget : parent->children())
    {
        if (auto focusWidget = getFocus(childWidget))
        {
            return focusWidget;
        }
    }

    return nullptr;
}

Dialog::Dialog(Wt::WObject* parent)
{
    rejectWhenEscapePressed();
    finished().connect(std::bind([this]() {
        if (result() == Wt::DialogCode::Accepted)
        {
            apply();
        }
        else if(result() == Wt::DialogCode::Rejected)
        {
            cancel();
        }
        delete this;
    }));
}


void Dialog::setButtons(stl_tools::flag_type<DialogButton> buttons)
{
    footer()->clear();

    if (buttons.contains(BTN_OK))
    {
        auto btnOk = footer()->addWidget(std::make_unique<Wt::WPushButton>("OK"));
        btnOk->setDefault(true);
        btnOk->clicked().connect([this](Wt::WMouseEvent){
            auto focused = getFocus();
            if (dynamic_cast<Wt::WTextArea*>(focused))
            {
                return;
            }
                        
            if (validate())
            {
                accept();
            }           
        });
    }

    if (buttons.contains(BTN_CANCEL))
    {
        auto btnCancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
        btnCancel->clicked().connect(this, &Wt::WDialog::reject);
    }

    if (buttons.contains(BTN_CLOSE))
    {
        auto btnClose = footer()->addWidget(std::make_unique<Wt::WPushButton>("Close"));
        btnClose->clicked().connect(this, &Wt::WDialog::reject);

        if (!buttons.contains(BTN_OK))
        {
            btnClose->setDefault(true);
        }
    }
}

Wt::WWidget* Dialog::getFocus() const
{
    return ::getFocus(contents());
}

bool Dialog::validate()
{
    return true;
}

void Dialog::apply()
{
}

void Dialog::cancel()
{
}
