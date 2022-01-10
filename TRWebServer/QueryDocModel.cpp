#include "stdafx.h"
#include "QueryDocModel.h"
#include "Application.h"

struct BuildQueryNodeRow: boost::static_visitor<std::unique_ptr<QueryNodeRow>>
{
    std::unique_ptr<QueryNodeRow> operator()(TR::XML::XmlCompositeQueryNode& queryNode) const
    {
        return std::make_unique<CompositeQueryNodeRow>(queryNode);
    }

    std::unique_ptr<QueryNodeRow> operator()(TR::XML::XmlConditionalQueryNode& queryNode) const
    {
        return std::make_unique<ConditionalQueryNodeRow>(queryNode);
    }
};

struct AppendInheritedConditions: boost::static_visitor<void>
{
    AppendInheritedConditions(QueryNodeRow& queryNodeRow):
        m_queryNodeRow(queryNodeRow)
    {
    }

    void operator()(const TR::XML::XmlCompositeQueryNode& baseQueryNode) const
    {
        auto childQueryNode = baseQueryNode.begin_child_node();
        auto& compositeRow = dynamic_cast<CompositeQueryNodeRow&>(m_queryNodeRow);
        for (size_t position = 0; position < compositeRow.getChildCount(); ++position, ++childQueryNode)
        {
            if (childQueryNode == baseQueryNode.end_child_node())
            {
                throw(std::logic_error("Invalid query document format"));
            }

            AppendInheritedConditions appender(compositeRow.getChildRow(position));
            visit_query_node(appender, *childQueryNode);
        }   
    }

    void operator()(const TR::XML::XmlConditionalQueryNode& baseQueryNode) const
    {
        auto& conditionalRow = dynamic_cast<ConditionalQueryNodeRow&>(m_queryNodeRow);
        for (auto& conditionNode : baseQueryNode.get_conditions())
        {
            conditionalRow.addChildRow(std::make_unique<ConditionNodeRow>(const_cast<TR::XML::XmlConditionNode&>(conditionNode), true));
        }
    }

    QueryNodeRow& m_queryNodeRow;
};

QueryNodeRow::QueryNodeRow(TR::XML::XmlQueryNode& queryNode):
    m_queryNode(queryNode)
{
}

const TR::XML::XmlQueryNode& QueryNodeRow::getNode() const
{
    return m_queryNode;
}

TR::XML::XmlQueryNode&  QueryNodeRow::getNode()
{
    return m_queryNode;
}

CompositeQueryNodeRow::CompositeQueryNodeRow(TR::XML::XmlCompositeQueryNode& queryNode):
    QueryNodeRow(queryNode)
{
    for (auto& childNode : queryNode.get_child_nodes())
    {
        addChildRow(visit_query_node(BuildQueryNodeRow(), childNode));
    }
}

size_t CompositeQueryNodeRow::getChildCount() const
{
    return BaseTreeModelRow::getChildCount();
}

QueryNodeRow& CompositeQueryNodeRow::getChildRow(size_t position)
{
    return static_cast<QueryNodeRow&>(BaseTreeModelRow::getChildRow(position));
}

ConditionalQueryNodeRow::ConditionalQueryNodeRow(TR::XML::XmlConditionalQueryNode& queryNode):
    QueryNodeRow(queryNode)
{
    for (auto& conditionNode : queryNode.get_conditions())
    {
        addChildRow(std::make_unique<ConditionNodeRow>(conditionNode));
    }
}

void ConditionalQueryNodeRow::addConditionNodeRow(std::unique_ptr<TR::XML::XmlConditionNode> conditionNode)
{
    auto& conditionalNode = static_cast<TR::XML::XmlConditionalQueryNode&>(m_queryNode);
    addChildRow(std::make_unique<ConditionNodeRow>(*conditionalNode.push_condition_front(std::move(conditionNode))), 0);
}

void ConditionalQueryNodeRow::moveUp(size_t conditionNodeRowPosition)
{   
    _ASSERT(conditionNodeRowPosition < getChildCount());
    if (conditionNodeRowPosition == 0)
    {
        return;
    }

    auto& nextRow = static_cast<ConditionNodeRow&>(getChildRow(conditionNodeRowPosition));
    auto& prevRow = static_cast<ConditionNodeRow&>(getChildRow(conditionNodeRowPosition - 1));

    xml_tools::swap_nodes(*nextRow.m_conditionNode, *prevRow.m_conditionNode);
    swapChildRows(conditionNodeRowPosition, conditionNodeRowPosition - 1);
}

void ConditionalQueryNodeRow::moveDown(size_t conditionNodeRowPosition)
{
    _ASSERT(conditionNodeRowPosition < getChildCount());
    if (conditionNodeRowPosition == getChildCount() - 1)
    {
        return;
    }

    auto& prevRow = static_cast<ConditionNodeRow&>(getChildRow(conditionNodeRowPosition));
    auto& nextRow = static_cast<ConditionNodeRow&>(getChildRow(conditionNodeRowPosition+1));
    if (nextRow.isInherited())
    {
        return;
    }
    
    xml_tools::swap_nodes(*prevRow.m_conditionNode, *nextRow.m_conditionNode);
    swapChildRows(conditionNodeRowPosition, conditionNodeRowPosition + 1);
}

ConditionNodeRow::ConditionNodeRow(TR::XML::XmlConditionNode& conditionNode, bool inherited):
    m_conditionNode(&conditionNode),
    m_inherited(inherited)
{
}

void ConditionNodeRow::moveUp()
{
    if (auto parentRow = static_cast<ConditionalQueryNodeRow*>(getParentRow()))
    {
        parentRow->moveUp(getPosition());
    }   
}

void ConditionNodeRow::moveDown()
{
    if (auto parentRow = static_cast<ConditionalQueryNodeRow*>(getParentRow()))
    {
        parentRow->moveDown(getPosition());
    }
}

const TR::XML::XmlConditionNode& ConditionNodeRow::getNode() const
{
    return *m_conditionNode;
}

TR::XML::XmlConditionNode& ConditionNodeRow::getNode()
{
    return *m_conditionNode;
}

bool ConditionNodeRow::isInherited() const
{
    return m_inherited;
}

void ConditionNodeRow::replaceNode(std::unique_ptr<TR::XML::XmlConditionNode> conditionNode)
{
    auto newConditionNode = conditionNode.get();
    xml_tools::replace_node(*m_conditionNode, std::move(conditionNode));
    m_conditionNode = newConditionNode;
    emitChanged();
}

std::wstring QueryDocColumn::getString(const BaseTreeModelRow& row) const
{
    if (auto queryNodeRow = dynamic_cast<const QueryNodeRow*>(&row))
    {
        return getString(queryNodeRow->getNode());
    }
    else if (auto conditionNodeRow = dynamic_cast<const ConditionNodeRow*>(&row))
    {
        return getString(conditionNodeRow->getNode());
    }
    else
    {
        _ASSERT(false);
    }
    return L"";
}

std::wstring QueryNameColumn::getName() const
{
    return L"Name";
}

std::wstring QueryNameColumn::getString(const TR::XML::XmlQueryNode& queryNode) const
{
    return queryNode.get_bound_def().get_caption();
}

std::wstring QueryNameColumn::getString(const TR::XML::XmlConditionNode& conditionNode) const
{
    return conditionNode.get_patterns()[0];
}

std::wstring QueryDescriptionColumn::getName() const
{
    return L"Description";
}

std::wstring QueryDescriptionColumn::getString(const TR::XML::XmlQueryNode& queryNode) const
{
    return queryNode.get_bound_def().get_description();
}

std::wstring QueryDescriptionColumn::getString(const TR::XML::XmlConditionNode& conditionNode) const
{
    return L"";
}

QueryDocModel::QueryDocModel(TR::XML::XmlQueryDoc& query_doc, const TR::XML::XmlQueryDoc* base_doc):
    m_doc(query_doc),
    m_base_doc(base_doc)
{
    
    auto queryNodeRow = visit_query_node(BuildQueryNodeRow(), *query_doc.get_root());
    if (base_doc)
    {
        AppendInheritedConditions appender(*queryNodeRow);
        visit_query_node(appender, *base_doc->get_root());
    }

    auto hiddenRoot = std::make_unique<BaseTreeModelRow>();
    hiddenRoot->addChildRow(std::move(queryNodeRow));
    reset(std::move(hiddenRoot));
}