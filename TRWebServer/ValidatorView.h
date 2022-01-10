#pragma once
#include "TRWebServer.h"
#include "QueryDocView.h"

class ValidatorItemDelegate: public Wt::WAbstractItemDelegate
{
public:
    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget *widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags);
};

class ValidatorView: public QueryDocView
{
public:
    ValidatorView(TR::XML::XmlQueryDoc& validator_doc, const TR::XML::XmlQueryDoc* base_validator_doc);
    ~ValidatorView();

private:
    std::shared_ptr<ValidatorItemDelegate> m_validatorItemDelegate;
};