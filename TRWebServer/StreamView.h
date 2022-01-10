#pragma once
#include "StandardTable.h"
#include "TRWebServer.h"
#include "MainFrame.h"

using StreamMessageRow = StandardModelRow<TR::StreamMessage>;
using StreamMessageColumn = StandardTableColumn<TR::StreamMessage>;

class MessageEventColumn: public StreamMessageColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const StreamMessageRow& row) const override;
};

class MessageDateColumn: public StreamMessageColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const StreamMessageRow& row) const override;
};

class MessageSeveretyColumn: public StreamMessageColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const StreamMessageRow& row) const override;
};

template<class XmlNodeType>
class MessageBodyColumn: public StreamMessageColumn
{
public:
    MessageBodyColumn(const TR::XML::XmlDefNode& defNode):
        m_defNode(defNode)
    {
    }

    virtual std::wstring getName() const override
    {
        return m_defNode.get_caption();
    }

    virtual std::wstring getString(const StreamMessageRow& row) const override
    {
        if (auto rowNode = getRowNode(row))
        {
            return getString(*rowNode);
        }
        return L"-";
    }

    virtual std::wstring getString(const XmlNodeType& node) const
    {
        return node.get_wtext();
    }

    virtual bool less(const StreamMessageRow& left, const StreamMessageRow& right) const override
    {
        auto rightNode = getRowNode(right);
        if (!rightNode)
        {
            return false;
        }

        auto leftNode = getRowNode(left);
        if (!leftNode)
        {
            return true;
        }

        return leftNode->get_value() < rightNode->get_value();
    }

    const XmlNodeType* getRowNode(const StreamMessageRow& row) const
    {
        auto body = row.getData().m_body;
        if (auto node = xml_tools::find_xpath_node(body->as_regular_doc(), make_xpath(m_defNode).c_str()))
        {
            return TR::XML::cast_regular_node<XmlNodeType>(static_cast<const TR::XML::XmlRegularNode*>(node));
        }
        return nullptr;
    }

protected:
    const TR::XML::XmlDefNode& m_defNode;
};

class BooleanMessageBodyColumn: public MessageBodyColumn<TR::XML::XmlBooleanNode>
{
public:
    BooleanMessageBodyColumn(const TR::XML::XmlBooleanDef& booleanDef);
    virtual std::wstring getString(const TR::XML::XmlBooleanNode& booleanNode) const override;
};

class EnumMessageBodyColumn: public MessageBodyColumn<TR::XML::XmlEnumNode>
{
public:
    EnumMessageBodyColumn(const TR::XML::XmlEnumDef& enumDef);
    virtual std::wstring getString(const TR::XML::XmlEnumNode& enumNode) const override;

private:
    std::vector<std::wstring> m_names;
};

class StreamView: public StandardTable<TR::StreamMessage>
{
public:
    enum ScrollPosition {NONE = 0, BOTTOM};

    StreamView(std::shared_ptr<const TR::XmlDefDoc> message_def);
    
    void            addMessage(TR::StreamMessage message);  
    ScrollPosition  getScrollPosition() const;
    void            scrollBottom();

private:
    void addBodyColumns(const TR::XML::XmlDefNode& def_node);
    std::shared_ptr<const TR::XmlDefDoc> m_message_def;
    ScrollPosition m_scrollPosition;
};