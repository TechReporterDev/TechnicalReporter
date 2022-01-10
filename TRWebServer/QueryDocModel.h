#pragma once
#include "TRWebServer.h"
#include "BaseTreeModel.h"

class ConditionNodeRow;
class QueryNodeRow: public BaseTreeModelRow
{
public:
    friend class QueryDocModel;
    QueryNodeRow(TR::XML::XmlQueryNode& queryNode);

    QueryNodeRow(const QueryNodeRow& queryNodeRow) = delete;
    QueryNodeRow& operator = (const QueryNodeRow& queryNodeRow) = delete;
    
    const TR::XML::XmlQueryNode&    getNode() const;
    TR::XML::XmlQueryNode&          getNode();

protected:  
    TR::XML::XmlQueryNode& m_queryNode;
};

class CompositeQueryNodeRow: public QueryNodeRow
{
public:
    CompositeQueryNodeRow(TR::XML::XmlCompositeQueryNode& queryNode);
    CompositeQueryNodeRow(const CompositeQueryNodeRow& queryNodeRow) = delete;
    CompositeQueryNodeRow& operator = (const CompositeQueryNodeRow& queryNodeRow) = delete;

    size_t          getChildCount() const;
    QueryNodeRow&   getChildRow(size_t position);
};

class ConditionalQueryNodeRow: public QueryNodeRow
{
public:
    friend class ConditionNodeRow;
    friend struct AppendInheritedConditions;

    ConditionalQueryNodeRow(TR::XML::XmlConditionalQueryNode& queryNode);

    ConditionalQueryNodeRow(const ConditionalQueryNodeRow& queryNodeRow) = delete;
    ConditionalQueryNodeRow& operator = (const ConditionalQueryNodeRow& queryNodeRow) = delete;

    void addConditionNodeRow(std::unique_ptr<TR::XML::XmlConditionNode> conditionNode);
    void moveUp(size_t conditionNodeRowPosition);
    void moveDown(size_t conditionNodeRowPosition);
};

class ConditionNodeRow: public BaseTreeModelRow
{
public:
    friend class ConditionalQueryNodeRow;
    ConditionNodeRow(TR::XML::XmlConditionNode& conditionNode, bool inherited = false);

    ConditionNodeRow(const ConditionNodeRow& conditionNodeRow) = delete;
    ConditionNodeRow& operator = (const ConditionNodeRow& conditionNodeRow) = delete;

    const TR::XML::XmlConditionNode&    getNode() const;
    TR::XML::XmlConditionNode&          getNode();
    bool                                isInherited() const;
    void                                replaceNode(std::unique_ptr<TR::XML::XmlConditionNode> conditionNode);

    void                                moveUp();
    void                                moveDown();

private:
    TR::XML::XmlConditionNode* m_conditionNode;
    bool m_inherited;
};

class QueryDocColumn: public BaseTreeModelColumn
{
public:
    virtual std::wstring    getString(const BaseTreeModelRow& row) const override;

private:
    virtual std::wstring    getString(const TR::XML::XmlQueryNode& queryNode) const = 0;
    virtual std::wstring    getString(const TR::XML::XmlConditionNode& conditionNode) const = 0;
};

class QueryNameColumn: public QueryDocColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const TR::XML::XmlQueryNode& queryNode) const override;
    virtual std::wstring    getString(const TR::XML::XmlConditionNode& conditionNode) const override;
};

class QueryDescriptionColumn: public QueryDocColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const TR::XML::XmlQueryNode& queryNode) const override;
    virtual std::wstring    getString(const TR::XML::XmlConditionNode& conditionNode) const override;
};

class QueryDocModel: public BaseTreeModel
{
public:
    friend class QueryDocView;

    QueryDocModel(TR::XML::XmlQueryDoc& query_doc, const TR::XML::XmlQueryDoc* base_doc = nullptr);
    QueryDocModel(const QueryDocModel&) = delete;
    QueryDocModel& operator = (const QueryDocModel&) = delete;
    virtual ~QueryDocModel(){}

    using BaseTreeModel::addColumn;
    TR::XML::XmlQueryDoc&               getQueryDoc();
    QueryNodeRow&                       getRootRow();
    
private:
    TR::XML::XmlQueryDoc& m_doc;
    const TR::XML::XmlQueryDoc* m_base_doc;
};