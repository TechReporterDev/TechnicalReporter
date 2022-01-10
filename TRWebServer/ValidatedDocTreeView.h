#pragma once
#include "TRWebServer.h"
#include "RegularDocTreeView.h"
class ValidatedDocModel;

class ValidatedDocTreeView: public RegularDocTreeView
{
public:
    ValidatedDocTreeView(std::unique_ptr<ValidatedDocModel> model);
    ~ValidatedDocTreeView();

protected:
    virtual std::wstring getRowStyle(RegularNodeRow& regNodeRow) const override;
};