#pragma once
#include "TRWebServer.h"
#include "QueryDocModel.h"

class QueryItemDelegate: public Wt::WAbstractItemDelegate
{
public:
    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget *widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags);
};

class QueryDocView: public Wt::WTreeView
{
public:
    QueryDocView(TR::XML::XmlQueryDoc& query_doc, const TR::XML::XmlQueryDoc* base_query_doc = nullptr);
    ~QueryDocView();

private:
    std::shared_ptr<QueryDocModel> m_model;
    std::shared_ptr<QueryItemDelegate> m_queryItemDelegate;
};