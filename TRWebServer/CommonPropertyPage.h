#pragma once
#include "TRWebServer.h"
#include "CommonProperty.h"
#include "PropertySheet.h"

class CommonPropertyPage: public PropertyPageWidget
{
public:
    CommonPropertyPage(const CommonProperty& commonProperty, int identity = 0);
    
    // WWidget override
    virtual void refresh() override;

protected:
    // PropertyPageWidget override
    virtual bool validate() override;

    CommonPropertyPage(Wt::WString caption, Wt::WString description, int identity = 0);
    void show(const CommonProperty& commonProperty);
    virtual void handleEvent(boost::any any);
};