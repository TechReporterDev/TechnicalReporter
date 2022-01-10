#include "stdafx.h"
#include "XmlDocTreeView.h"
#include "XmlDocModel.h"
#include "Application.h"

XmlDocTreeView::XmlDocTreeView(std::unique_ptr<XmlDocModel> model):
    m_model(std::move(model))
{
    setModel(std::shared_ptr<Wt::WAbstractItemModel>(m_model, m_model.get()));
    expandToDepth(3);
}

XmlDocTreeView::~XmlDocTreeView()
{
}

XmlDocModel* XmlDocTreeView::getXmlDocModel()
{
    return m_model.get();
}