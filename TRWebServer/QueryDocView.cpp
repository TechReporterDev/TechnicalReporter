#include "stdafx.h"
#include "QueryDocView.h"
#include "QueryConditionDlg.h"
#include "Application.h"

namespace {
void showAddConditionDialog(ConditionalQueryNodeRow& conditionalQueryNodeRow)
{   
    auto dlg = new QueryConditionDlg(TR::XML::create_condition(TR::XML::XmlConditionNode::Action::ACCEPT, TR::XML::XmlConditionNode::Predicate::EQUAL, {L""}));
    dlg->setOnOK([&](std::unique_ptr<TR::XML::XmlConditionNode> conditionNode){
        conditionalQueryNodeRow.addConditionNodeRow(std::move(conditionNode));
    });

    dlg->setWidth(300);
    dlg->show();
}

void showUpdateConditionDialog(ConditionNodeRow& conditionNodeRow)
{
    auto dlg = new QueryConditionDlg(xml_tools::clone_node(conditionNodeRow.getNode()));
    dlg->setOnOK([&](std::unique_ptr<TR::XML::XmlConditionNode> conditionNode){
        conditionNodeRow.replaceNode(std::move(conditionNode));
    });

    dlg->setWidth(300);
    dlg->show();
}

std::wstring getConditionString(const TR::XML::XmlConditionNode& conditionNode)
{
    if (auto name = conditionNode.get_name())
    {
        if (!name->empty())
        {
            return stl_tools::utf8_to_ucs(*name);
        }
    }
        
    std::wstring str;
    switch (conditionNode.get_action())
    {
    case TR::XML::XmlConditionNode::Action::ACCEPT:
        str += L"accept ";
        break;

    case TR::XML::XmlConditionNode::Action::REJECT:
        str += L"reject ";
        break;

    default:
        _ASSERT(false);
    }

    switch (conditionNode.get_predicate())
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

    str += conditionNode.get_patterns()[0];
    return str;
}

} //namespace {

std::unique_ptr<Wt::WWidget> QueryItemDelegate::update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
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
        
        auto actionCheck = itemWidget->addWidget(std::make_unique<Wt::WCheckBox>(queryNode.get_bound_def().get_caption()));     
        actionCheck->addStyleClass("tr-inplace-checkbox");
        actionCheck->setChecked(queryNode.get_action() == TR::XML::XmlQueryNode::Action::ACCEPT);
        actionCheck->changed().connect([&queryNode, actionCheck](Wt::NoClass) mutable {
            queryNode.set_action(actionCheck->isChecked() ? TR::XML::XmlQueryNode::Action::ACCEPT : TR::XML::XmlQueryNode::Action::REJECT);
        });

        if (auto conditionalFilterNodeRow = dynamic_cast<ConditionalQueryNodeRow*>(queryNodeRow))
        {
            auto addBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Add"));          
            addBtn->setFloatSide(Wt::Side::Right);
            addBtn->setStyleClass("btn-xs");
            addBtn->clicked().connect([conditionalFilterNodeRow](Wt::WMouseEvent){
                showAddConditionDialog(*conditionalFilterNodeRow);
            });
        }
    }
    else if (auto conditionNodeRow = dynamic_cast<ConditionNodeRow*>(&nodeRow))
    {
        auto& conditionNode = conditionNodeRow->getNode();
        itemWidget->addWidget(std::make_unique<Wt::WText>(getConditionString(conditionNode)));

        auto editBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Edit"));
        editBtn->setFloatSide(Wt::Side::Right);
        editBtn->setStyleClass("btn-xs");
        editBtn->setMinimumSize(25, 0);
        editBtn->clicked().connect([conditionNodeRow](Wt::WMouseEvent) {
            showUpdateConditionDialog(*conditionNodeRow);
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
    else
    {
        _ASSERT(false);
    }

    return itemWidget;
}

QueryDocView::QueryDocView(TR::XML::XmlQueryDoc& query_doc, const TR::XML::XmlQueryDoc* base_query_doc)
{
    m_queryItemDelegate = std::make_shared<QueryItemDelegate>();
    m_model = std::make_shared<QueryDocModel>(query_doc, base_query_doc);
    m_model->addColumn(std::make_unique<QueryNameColumn>());
    m_model->addColumn(std::make_unique<QueryDescriptionColumn>());
    setModel(std::shared_ptr<Wt::WAbstractItemModel>(m_model, m_model.get()));

    setItemDelegateForColumn(0, m_queryItemDelegate);
    setRowHeight(23);
    setLineHeight(23);
    expandToDepth(3);
    setColumnWidth(1, 500);
}

QueryDocView::~QueryDocView()
{
}