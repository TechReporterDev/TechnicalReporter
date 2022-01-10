#pragma once
#include "TRWebServer.h"
#include "XmlDiffTreeView.h"
class RegularDiffModel;

class RegularDiffTreeView: public XmlDiffTreeView
{
public:
    RegularDiffTreeView(std::unique_ptr<RegularDiffModel> model);
    ~RegularDiffTreeView();
};