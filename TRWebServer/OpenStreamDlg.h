#pragma once
#include "TRWebServer.h"
#include "PropertyDialog.h"

class OpenStreamDlg: public PropertyDialog
{
public:
    enum Period {LAST_DAY = 0, LAST_WEEK, CUSTOM_PERIOD};
    OpenStreamDlg(TR::StreamInfo streamInfo);

    // Dialog override
    virtual void apply() override;

private:
    TR::StreamInfo m_streamInfo;
    
    Period m_period;
    std::tm m_startDate;
    std::tm m_stopDate;
};