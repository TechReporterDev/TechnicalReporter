#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class Dialog: public Wt::WDialog
{
public:
    enum DialogButton {BTN_OK = 0x01, BTN_CANCEL = 0x02, BTN_CLOSE = 0x04};
    Dialog(Wt::WObject* parent = 0);

protected:
    void setButtons(stl_tools::flag_type<DialogButton> buttons);
    Wt::WWidget* getFocus() const;

    // override
    virtual bool validate();
    virtual void apply();
    virtual void cancel();
};

Wt::WWidget* getFocus(Wt::WWidget* parent_);
