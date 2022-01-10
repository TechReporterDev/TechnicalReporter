#include "stdafx.h"
#include "StreamFilterView.h"
#include "QueryConditionDlg.h"
#include "InventoryTable.h"
#include "Application.h"

namespace {
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

class StreamFilterConditionsView: public Wt::WContainerWidget
{
public:
    StreamFilterConditionsView(TR::XML::XmlConditionalQueryNode& parentNode):
        m_parentNode(parentNode),
        m_conditions(nullptr),
        m_emptyMessage(nullptr)
    {
        addStyleClass("tr-section");
        auto header = addWidget(std::make_unique<Wt::WContainerWidget>());
        auto ds = header->decorationStyle();
        ds.setBackgroundColor(Wt::WColor(0xF5, 0xF5, 0xF5));
        header->setDecorationStyle(ds);     

        auto hbox = header->setLayout(std::make_unique<Wt::WHBoxLayout>());
        hbox->addWidget(std::make_unique<Wt::WText>(m_parentNode.get_bound_def().get_caption()), 1);

        auto toolbar = hbox->addWidget(std::make_unique<Wt::WToolBar>());

        auto addBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Add");
        addBtn->clicked().connect([this](Wt::WMouseEvent){
            auto dlg = new QueryConditionDlg(TR::XML::create_condition(TR::XML::XmlConditionNode::Action::ACCEPT, TR::XML::XmlConditionNode::Predicate::EQUAL, {L""}));
            dlg->setOnOK([this](std::unique_ptr<TR::XML::XmlConditionNode> conditionNode){
                addCondition(std::move(conditionNode));
            });

            dlg->setWidth(300);
            dlg->show();
        });     

        m_conditions = addWidget(std::make_unique<InventoryTable>());
        m_conditions->setEmptyMessage(L"Click 'Add' to add condition for this column");
        m_conditions->addTool("Move", [this](int index){
            m_conditions->setMoveMode([this,index](int target){
                if (index == target)
                {
                    return;
                }
                moveCondition(index, target);
            });         
        });

        m_conditions->addTool("Edit", [this](int index){
            auto position = std::next(m_parentNode.begin_condition(), index);
            auto dlg = new QueryConditionDlg(xml_tools::clone_node(*position));
            dlg->setOnOK([this, position, index](std::unique_ptr<TR::XML::XmlConditionNode> conditionNode){
                updateCondition(index, std::move(conditionNode));
            });

            dlg->setWidth(300);
            dlg->show();
        }, true);

        m_conditions->addTool("Delete", [this](int index){
            deleteCondition(index);
        });

        size_t position = 0;
        for (auto& conditionNode : m_parentNode.get_conditions())
        {
            m_conditions->setItem(position++, getConditionString(conditionNode), L"", &conditionNode);
        }
    }


private:
    void addCondition(std::unique_ptr<TR::XML::XmlConditionNode> conditionNode)
    {
        auto position = m_parentNode.push_condition_front(std::move(conditionNode));
        m_conditions->insertItem(0, getConditionString(*position), L"", &*position);
    }

    void updateCondition(int index, std::unique_ptr<TR::XML::XmlConditionNode> conditionNode)
    {
        auto& condition = *conditionNode;
        xml_tools::replace_node(*std::next(m_parentNode.begin_condition(), index), std::move(conditionNode));
        m_conditions->setItem(index, getConditionString(condition), L"", &condition);
    }

    void deleteCondition(int index)
    {
        auto position = std::next(m_parentNode.begin_condition(), index);
        m_parentNode.remove_condition(position);
        m_conditions->deleteItem(index);
    }

    void moveCondition(int from, int to)
    {
        auto& sibling = *std::next(m_parentNode.begin_condition(), to);
        auto& conditionNode = *std::next(m_parentNode.begin_condition(), from);
        
        (from > to) ? 
            xml_tools::insert_node(sibling, xml_tools::remove_node(conditionNode)):
            xml_tools::insert_after(sibling, xml_tools::remove_node(conditionNode));
        
        m_conditions->deleteItem(from);
        m_conditions->insertItem(to, getConditionString(conditionNode), L"", &conditionNode);
    }   

    TR::XML::XmlConditionalQueryNode& m_parentNode;
    InventoryTable* m_conditions;
    Wt::WText* m_emptyMessage;
};
} //namespace {

StreamFilterView::StreamFilterView(TR::XML::XmlQueryDoc& filterDoc):
    m_filterDoc(filterDoc)
{
    setOverflow(Wt::Overflow::Auto);
    showFilterNode(*filterDoc.get_root());
}

void StreamFilterView::showFilterNode(TR::XML::XmlQueryNode& filterNode)
{
    if (auto conditionalNode = TR::XML::cast_query_node<TR::XML::XmlConditionalQueryNode>(&filterNode))
    {
        addWidget(std::make_unique<StreamFilterConditionsView>(*conditionalNode));
    }
    else if (auto compositeNode = TR::XML::cast_query_node<TR::XML::XmlCompositeQueryNode>(&filterNode))
    {
        for (auto& childNode : compositeNode->get_child_nodes())
        {
            showFilterNode(childNode);
        }
    }
}
