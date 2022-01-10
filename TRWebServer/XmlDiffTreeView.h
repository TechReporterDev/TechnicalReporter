#pragma once
#include "TRWebServer.h"
class XmlDiffModel;

class XmlDiffTreeView: public Wt::WTreeView
{
public:
    XmlDiffTreeView(std::shared_ptr<XmlDiffModel> model);
    ~XmlDiffTreeView();

private:
    std::shared_ptr<XmlDiffModel> m_model;
};