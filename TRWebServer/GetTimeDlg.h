#pragma once
#include "TRWebServer.h"
#include "PropertyDialog.h"

class GetTimeDlg: public PropertyDialog
{
public:
    using OnOK = std::function<void(time_t time)>;
    
    GetTimeDlg(const Wt::WString& caption, Wt::WObject* parent = 0);
    void show(OnOK onOK);
    
protected:
    virtual void apply() override;

    std::tm m_date;
    std::tm m_time;
    OnOK m_onOK;
};
