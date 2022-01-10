#include "stdafx.h"
#include "PropertyNodeValue.h"
#include "Application.h"

template<class T>
class XmlTrivialNodeValue: public SingleValue<T>
{
public:
    XmlTrivialNodeValue(TR::XML::XmlTrivialNode<T>& trivialNode):
        m_trivialNode(trivialNode)
    {
    }

    virtual std::shared_ptr<Value> clone() const override
    {
        return staticValue(get());
    }

    virtual T get() const override
    {
        return m_trivialNode.get_value();
    }

    virtual void set(T value) override
    {
        m_trivialNode.set_value(value);
    }

private:
    TR::XML::XmlTrivialNode<T>& m_trivialNode;
};

std::shared_ptr<SingleValue<bool>> createValue(TR::XML::XmlCheckNode& checkNode)
{
    return std::make_shared<XmlTrivialNodeValue<bool>>(checkNode.get_value_node());
}

std::shared_ptr<SingleValue<int>> createValue(TR::XML::XmlNumberNode& numberNode)
{
    return std::make_shared<XmlTrivialNodeValue<int>>(numberNode.get_value_node());
}

std::shared_ptr<SingleValue<std::wstring>> createValue(TR::XML::XmlStringLineNode& stringLineNode)
{
    return std::make_shared<XmlTrivialNodeValue<std::wstring>>(stringLineNode.get_value_node());
}

std::shared_ptr<SingleValue<std::wstring>> createValue(TR::XML::XmlTextAreaNode& textAreaNode)
{
    return std::make_shared<XmlTrivialNodeValue<std::wstring>>(textAreaNode.get_value_node());
}

std::shared_ptr<SingleValue<int>> createValue(TR::XML::XmlSelectorNode& selectorNode)
{
    return std::make_shared<XmlTrivialNodeValue<int>>(selectorNode.get_value_node());
}

struct CreateCollectionMemberValue: boost::static_visitor<std::shared_ptr<Value>>
{
    template<class XmlPropertyNode>
    std::shared_ptr<Value> operator()(XmlPropertyNode& memberNode) const
    {
        return createValue(memberNode);
    }
};

class XmlPropertyCollectionNodeValue: public ValueCollection
{
public:
    XmlPropertyCollectionNodeValue(TR::XML::XmlPropertyCollectionNode& collectionNode):
        m_collectionNode(collectionNode)
    {
    }

    virtual std::shared_ptr<Value> clone() const override
    {
        auto valueCollection = std::make_shared<ComposedCollection>();
        for (auto& propertyNode : m_collectionNode.get_properties())
        {
            auto memberValue = visit_property_node(CreateCollectionMemberValue(), propertyNode);
            valueCollection->appendMember(memberValue->clone());
        }
        return valueCollection;
    }

    virtual size_t size() const override
    {
        return boost::distance(m_collectionNode.get_properties());
    }

    virtual Member getMember(size_t position) const override
    {
        auto& memberProperty = *std::next(m_collectionNode.begin_property(), position);
        return visit_property_node(CreateCollectionMemberValue(), memberProperty);
    }

private:
    TR::XML::XmlPropertyCollectionNode& m_collectionNode;
};

std::shared_ptr<ValueCollection> createValue(TR::XML::XmlPropertyCollectionNode& collectionNode)
{
    return std::make_shared<XmlPropertyCollectionNodeValue>(collectionNode);
}

class XmlPropertyListNodeArray: public ValueArray<ValueCollection>
{
public:
    XmlPropertyListNodeArray(TR::XML::XmlPropertyListNode& listNode):
        m_listNode(listNode)
    {
    }

    virtual std::shared_ptr<Value> clone() const override
    {
        auto selfArray = std::make_shared<SelfArray<ValueCollection>>();
        for (auto& propertyNode : m_listNode.get_properties())
        {
            auto propertyValue = createValue(propertyNode);
            selfArray->addItem(selfArray->size(), *std::dynamic_pointer_cast<ValueCollection>(propertyValue->clone()));
        }
        return selfArray;
    }

    virtual size_t size() const override
    {
        return boost::distance(m_listNode.get_properties());
    }

    virtual Item getItem(size_t position) const override
    {
        auto& propertyNode = *std::next(m_listNode.begin_property(), position);
        return std::make_shared<XmlPropertyCollectionNodeValue>(propertyNode);
    }

    virtual void addItem(size_t position, const ValueType& value) override
    {
        auto where_ = std::next(m_listNode.begin_property(), position);
        auto& propertyNode = *m_listNode.insert_property(where_);
        XmlPropertyCollectionNodeValue(propertyNode).assign(value);
    }

    virtual void removeItem(size_t position) override
    {
        auto where_ = std::next(m_listNode.begin_property(), position);
        m_listNode.remove_property(where_);
    }

private:
    TR::XML::XmlPropertyListNode& m_listNode;
};

std::shared_ptr<ValueArray<ValueCollection>> createValue(TR::XML::XmlPropertyListNode& listNode)
{
    return std::make_shared<XmlPropertyListNodeArray>(listNode);
}