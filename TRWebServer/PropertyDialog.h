#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "Dialog.h"
#include "PropertySheet.h"

class PropertyDialog: public Dialog
{
protected:
    PropertyDialog(const Wt::WString& title, Wt::WObject *parent = 0);
    void init(std::unique_ptr<PropertyPageWidget> propertyPage);

    // Dialog override
    virtual bool validate() override;
};