#pragma once
#include "TRWebServer.h"
class XmlDocModel;

class XmlDocTreeView: public Wt::WTreeView
{
public:
    XmlDocTreeView(std::unique_ptr<XmlDocModel> model);
    ~XmlDocTreeView();
    XmlDocModel* getXmlDocModel();

private:
    std::shared_ptr<XmlDocModel> m_model;
};