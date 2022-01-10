#pragma once
#include "TRWebServer.h"

class ValidatedDocTableView;
class ValidatedNodeTableView: public Wt::WAbstractItemDelegate
{
public:
    ValidatedNodeTableView(ValidatedDocTableView& tableView);
    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget *widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags) override;

private:
    ValidatedDocTableView& m_tableView;
};

class ValidatedDocTableView: public Wt::WTableView
{
public:
    friend class ValidatedNodeTableView;
    ValidatedDocTableView(const TR::XML::XmlRegularDoc& doc);
    ~ValidatedDocTableView();

private:
    const TR::XML::XmlRegularDoc& m_doc;
    std::shared_ptr<Wt::WStandardItemModel> m_model;
    TR::XML::XMLRegularTable m_regularTable;
    std::shared_ptr<ValidatedNodeTableView> m_validatedNodeView;
};