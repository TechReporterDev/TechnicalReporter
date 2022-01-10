#include "stdafx.h"
#include "ValidatedDocTreeView.h"
#include "ValidatedDocModel.h"
#include "Application.h"

ValidatedDocTreeView::ValidatedDocTreeView(std::unique_ptr<ValidatedDocModel> model):
    RegularDocTreeView(std::move(model))
{
}

ValidatedDocTreeView::~ValidatedDocTreeView()
{
}

std::wstring ValidatedDocTreeView::getRowStyle(RegularNodeRow& regNodeRow) const
{
    auto& validated = dynamic_cast<ValidatedAdditive&>(regNodeRow);
    if (!validated.isRecursiveValid())
    {
        return L"tr-invalid-cell";  
    }
    return L"";
}