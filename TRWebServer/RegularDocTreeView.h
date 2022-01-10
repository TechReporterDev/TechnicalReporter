#pragma once
#include "TRWebServer.h"
#include "XmlDocTreeView.h"
class RegularDocModel;
class RegularNodeRow;

class RegularDocTreeView: public XmlDocTreeView
{
public:
    class RegularNodeRowDelegate;
    using OnActionClick = std::function<void(const TR::XML::XmlRegularNode* context_node, const TR::XML::META::XmlActionNode* action_node)>;

    RegularDocTreeView(std::unique_ptr<RegularDocModel> m_model);
    ~RegularDocTreeView();
    RegularDocModel*        getRegularDocModel();
    void                    connectActionClick(OnActionClick onActionClick);

protected:
    virtual std::wstring    getRowStyle(RegularNodeRow& regNodeRow) const;
    virtual void            onActionClick(const TR::XML::XmlRegularNode* context_node, const TR::XML::META::XmlActionNode* action_node);

private:
    OnActionClick m_onActionClick;
};