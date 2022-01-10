#include "stdafx.h"
#include "PropertyDialog.h"
#include "Application.h"

PropertyDialog::PropertyDialog(const Wt::WString& title, Wt::WObject* parent):
    Dialog(parent)
{
    setWindowTitle(title);
    rejectWhenEscapePressed();  
    setButtons(stl_tools::flag | BTN_OK | BTN_CANCEL);
}

void PropertyDialog::init(std::unique_ptr<PropertyPageWidget> propertyPage)
{
    contents()->setPadding(5);
    auto propertySheet = contents()->addWidget(std::make_unique<PropertySheetWidget>());
    propertySheet->pushPage(std::move(propertyPage));
}

bool PropertyDialog::validate()
{
    return validatePropertyPages(this);
}