#include "stdafx.h"
#include "ValidatorView.h"
#include "PropertyDialog.h"
#include "CommonPropertyPage.h"
#include "CustomValue.h"
#include "StaticValue.h"
#include "Application.h"
#include <boost/algorithm/string.hpp>

namespace {

class EditValidationDlg: public PropertyDialog
{
public:
    typedef std::function<void(std::unique_ptr<TR::XML::XmlValidationNode>)> OnOK;

    EditValidationDlg(std::unique_ptr<TR::XML::XmlValidationNode> valdationNode):
        PropertyDialog(L"Validation"),
        m_valdationNode(std::move(valdationNode))
    {
        for (auto& pattern : m_valdationNode->get_patterns())
        {
            if (!m_patterns.empty())
            {
                m_patterns.append(L"\n");
            }
            m_patterns.append(pattern);
        }

        init(std::make_unique<CommonPropertyPage>(CommonProperty{L"", L"Setup validation condition",
            StringProperty(L"Comment", L"", customValue<std::wstring>(
                [this]{
                    if (auto comment = m_valdationNode->get_comment())
                    {
                        return stl_tools::utf8_to_ucs(*comment);
                    }
                    return std::wstring(L""); 
                },
                [this](const std::wstring& value){ m_valdationNode->set_comment(stl_tools::ucs_to_utf8(value)); })
            ),
            SelectorProperty(L"Decision", L"", {L"Valid", L"Invalid"}, customValue<int>(
                [this]{ return int(m_valdationNode->get_decision()); },
                [this](int value){ m_valdationNode->set_decision(TR::XML::XmlValidationNode::Decision(value)); })
            ),

            SelectorProperty(L"Predicate", L"", {L"Equal", L"Match", L"Less", L"More"}, customValue<int>(
                [this]{ return int(m_valdationNode->get_predicate()); },
                [this](int value){ m_valdationNode->set_predicate(TR::XML::XmlConditionNode::Predicate(value)); })
            ),

            TextProperty(L"Patterns", L"", 3, refValue(m_patterns))

        }, 0));
    }

    void setOnOK(OnOK onOK)
    {
        _ASSERT(!m_onOK);
        m_onOK = onOK;
    }   

    virtual void apply() override
    {
        std::vector<std::wstring> patterns;
        boost::split(patterns, m_patterns, boost::is_any_of(L"\r\n"), boost::algorithm::token_compress_on);
        m_valdationNode->set_patterns(patterns);

        _ASSERT(m_onOK);
        m_onOK(std::move(m_valdationNode));
    }

private:
    void showPatterns(const std::vector<std::wstring>& patterns);
    std::unique_ptr<TR::XML::XmlValidationNode> m_valdationNode;
    OnOK m_onOK;

    std::wstring m_patterns;
};

void showAddValidationDialog(ConditionalQueryNodeRow& conditionalQueryNodeRow)
{
    auto dlg = new EditValidationDlg(TR::XML::create_validation(TR::XML::XmlValidationNode::VALID, TR::XML::XmlConditionNode::Predicate::EQUAL, {L""}, ""));
    dlg->setOnOK([&](std::unique_ptr<TR::XML::XmlValidationNode> validationNode){
        conditionalQueryNodeRow.addConditionNodeRow(std::move(validationNode));
    });

    dlg->setWidth(300);
    dlg->show();
}

void showEditValidationDialog(ConditionNodeRow& conditionNodeRow)
{
    auto dlg = new EditValidationDlg(xml_tools::clone_node(static_cast<TR::XML::XmlValidationNode&>(conditionNodeRow.getNode())));
    dlg->setOnOK([&](std::unique_ptr<TR::XML::XmlValidationNode> validationNode){
        conditionNodeRow.replaceNode(std::move(validationNode));
    });

    dlg->setWidth(300);
    dlg->show();
}

std::wstring getValidationString(const TR::XML::XmlValidationNode& validationNode)
{
    if (auto comment = validationNode.get_comment())
    {
        if (!comment->empty())
        {
            return stl_tools::utf8_to_ucs(*comment);
        }
    }

    std::wstring str;
    switch (validationNode.get_decision())
    {
    case TR::XML::XmlValidationNode::VALID:
        str += L"valid ";
        break;

    case TR::XML::XmlValidationNode::INVALID:
        str += L"invalid ";
        break;

    default:
        _ASSERT(false);
    }

    switch (validationNode.get_predicate())
    {
    case TR::XML::XmlConditionNode::Predicate::EQUAL:
        str += L"equal ";
        break;

    case TR::XML::XmlConditionNode::Predicate::LESS:
        str += L"less ";
        break;

    case TR::XML::XmlConditionNode::Predicate::MORE:
        str += L"more ";
        break;

    case TR::XML::XmlConditionNode::Predicate::MATCH:
        str += L"match ";
        break;

    default:
        _ASSERT(false);
    }

    str += validationNode.get_patterns()[0];
    return str;
}

} //namespace {

std::unique_ptr<Wt::WWidget> ValidatorItemDelegate::update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
{
    if (widget)
    {
        //delete widget;
    }

    auto itemWidget = std::make_unique<Wt::WContainerWidget>();
    auto& nodeRow = getRowFromIndex(index);
    if (auto queryNodeRow = dynamic_cast<QueryNodeRow*>(&nodeRow))
    {
        auto& queryNode = queryNodeRow->getNode();
        itemWidget->addWidget(std::make_unique<Wt::WText>(queryNode.get_bound_def().get_caption()));
        if (auto conditionalQueryNodeRow = dynamic_cast<ConditionalQueryNodeRow*>(queryNodeRow))
        {
            auto addBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Add"));
            addBtn->setFloatSide(Wt::Side::Right);
            addBtn->setStyleClass("btn-xs");
            addBtn->clicked().connect([conditionalQueryNodeRow](Wt::WMouseEvent){
                showAddValidationDialog(*conditionalQueryNodeRow);
            });
        }
    }
    else if (auto conditionNodeRow = dynamic_cast<ConditionNodeRow*>(&nodeRow))
    {
        auto& validationNode = static_cast<TR::XML::XmlValidationNode&>(conditionNodeRow->getNode());

        auto validationString = getValidationString(validationNode);
        if (conditionNodeRow->isInherited())
        {
            itemWidget->addWidget(std::make_unique<Wt::WText>(validationString + L"(inherited)"));
        }
        else
        {
            itemWidget->addWidget(std::make_unique<Wt::WText>(validationString));
            auto editBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Edit"));
                editBtn->setFloatSide(Wt::Side::Right);
            editBtn->setStyleClass("btn-xs");
            editBtn->setMinimumSize(25, 0);
            editBtn->clicked().connect([conditionNodeRow](Wt::WMouseEvent) {
                showEditValidationDialog(*conditionNodeRow);
            });

            auto downBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Down"));
            downBtn->setFloatSide(Wt::Side::Right);
            downBtn->setStyleClass("btn-xs");
            downBtn->setMinimumSize(25, 0);
            downBtn->clicked().connect([conditionNodeRow](Wt::WMouseEvent) {
                conditionNodeRow->moveDown();
            });

            auto upBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Up"));
            upBtn->setFloatSide(Wt::Side::Right);
            upBtn->setStyleClass("btn-xs");
            upBtn->setMinimumSize(25, 0);
            upBtn->clicked().connect([conditionNodeRow](Wt::WMouseEvent) {
                conditionNodeRow->moveUp();
            });                     
        }
    }
    else
    {
        _ASSERT(false);
    }

    return itemWidget;
}

ValidatorView::ValidatorView(TR::XML::XmlQueryDoc& validator_doc, const TR::XML::XmlQueryDoc* base_validator_doc):
    QueryDocView(validator_doc, base_validator_doc),
    m_validatorItemDelegate(std::make_shared<ValidatorItemDelegate>())
{
    setItemDelegateForColumn(0, m_validatorItemDelegate);
    setRowHeight(23);
    setLineHeight(23);
}

ValidatorView::~ValidatorView()
{
}