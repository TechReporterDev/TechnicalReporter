#pragma once
#include "TRWebServer.h"
#include "PropertySheet.h"

class PropertyWizard: public Wt::WDialog
{
protected:
    PropertyWizard(const Wt::WString& windowTitle);

    void                                            init(std::unique_ptr<PropertyPageWidget> firstPage);
    virtual std::unique_ptr<PropertyPageWidget>     nextPage(int currentPageIdentity) = 0;
    virtual bool                                    isFinal(int currentPageIdentity) = 0;
    virtual void                                    onFinish() = 0;

private:
    PropertySheetWidget* m_propertySheet;
    Wt::WPushButton* m_btnNext;
    Wt::WPushButton* m_btnBack;
};