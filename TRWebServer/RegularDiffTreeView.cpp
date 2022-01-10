#include "stdafx.h"
#include "RegularDiffTreeView.h"
#include "RegularDiffModel.h"
#include "Application.h"

RegularDiffTreeView::RegularDiffTreeView(std::unique_ptr<RegularDiffModel> model):
    XmlDiffTreeView(std::unique_ptr<XmlDiffModel>(model.release()))
{
    expandToDepth(3);
}

RegularDiffTreeView::~RegularDiffTreeView()
{
}