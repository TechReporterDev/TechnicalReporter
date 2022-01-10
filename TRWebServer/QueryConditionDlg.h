#pragma once
#include "TRWebServer.h"
#include "PropertyDialog.h"

class QueryConditionDlg: public PropertyDialog
{
public:
    typedef std::function<void(std::unique_ptr<TR::XML::XmlConditionNode>)> OnOK;
    
    QueryConditionDlg(std::unique_ptr<TR::XML::XmlConditionNode> conditionNode);
    void setOnOK(OnOK onOK);

private:
    virtual void apply() override;
    
    std::unique_ptr<TR::XML::XmlConditionNode> m_conditionNode;
    TR::XML::XmlConditionNode::Predicate m_predicate;
    TR::XML::XmlConditionNode::Action m_action;
    std::wstring m_patterns;

    OnOK m_onOK;    
};

