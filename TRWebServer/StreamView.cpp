#include "stdafx.h"
#include "StreamView.h"
#include "Application.h"
#undef max

std::wstring MessageEventColumn::getName() const
{
    return L"Event";
}

std::wstring MessageEventColumn::getString(const StreamMessageRow& row) const
{
    return row.getData().m_event;
}

std::wstring MessageDateColumn::getName() const
{
    return L"Date";
}

std::wstring MessageDateColumn::getString(const StreamMessageRow& row) const
{
    return Application::formatTime(row.getData().m_time);
}

std::wstring MessageSeveretyColumn::getName() const
{
    return L"Severety";
}

std::wstring MessageSeveretyColumn::getString(const StreamMessageRow& row) const
{
    switch (row.getData().m_severety)
    {
    case TR::StreamMessage::Severety::INFO:
        return L"Info";

    case TR::StreamMessage::Severety::WARNING:
        return L"Warning";

    case TR::StreamMessage::Severety::FAILED:
        return L"Failed";

    default:
        _ASSERT(false);
    }
    return L"Unknown";
}

BooleanMessageBodyColumn::BooleanMessageBodyColumn(const TR::XML::XmlBooleanDef& booleanDef):
    MessageBodyColumn<TR::XML::XmlBooleanNode>(booleanDef)
{
}

std::wstring BooleanMessageBodyColumn::getString(const TR::XML::XmlBooleanNode& booleanNode) const
{
    if (booleanNode.get_value())
    {
        return L"True";
    }
    return L"False";
};

EnumMessageBodyColumn::EnumMessageBodyColumn(const TR::XML::XmlEnumDef& enumDef):
    MessageBodyColumn<TR::XML::XmlEnumNode>(enumDef)
{
    m_names = enumDef.get_names();
}

std::wstring EnumMessageBodyColumn::getString(const TR::XML::XmlEnumNode& enumNode) const
{
    return m_names.at(enumNode.get_value());
}

struct MakeMessageBodyColumn: boost::static_visitor<std::unique_ptr<StreamMessageColumn>>
{
    std::unique_ptr<StreamMessageColumn> operator()(const TR::XML::XmlStringDef& stringDef) const
    {
        return std::make_unique<MessageBodyColumn<TR::XML::XmlStringNode>>(stringDef);
    }

    std::unique_ptr<StreamMessageColumn> operator()(const TR::XML::XmlIntegerDef& integerDef) const
    {
        return std::make_unique<MessageBodyColumn<TR::XML::XmlIntegerNode>>(integerDef);
    }

    std::unique_ptr<StreamMessageColumn> operator()(const TR::XML::XmlBooleanDef& booleanDef) const
    {
        return std::make_unique<BooleanMessageBodyColumn>(booleanDef);
    }

    std::unique_ptr<StreamMessageColumn> operator()(const TR::XML::XmlEnumDef& enumDef) const
    {
        return std::make_unique<EnumMessageBodyColumn>(enumDef);
    }

    std::unique_ptr<StreamMessageColumn> operator()(const TR::XML::XmlDefNode&) const
    {
        throw std::logic_error("Invalid column type");
    }
};

StreamView::StreamView(std::shared_ptr<const TR::XmlDefDoc> message_def):
    m_message_def(message_def),
    m_scrollPosition(BOTTOM)
{
    addColumn(std::make_unique<MessageDateColumn>());
    addColumn(std::make_unique<MessageSeveretyColumn>());   
    addBodyColumns(*m_message_def->get_root_def());

    if (int bodyColumnCount = getColumnCount() - 2)
    {
        int bodyColumnWidth = std::max(150, 1000 / bodyColumnCount);
        for (int column = 0; column < bodyColumnCount; ++column)
        {
            setColumnWidth(column + 2, bodyColumnWidth);
        }       
    }

    scrolled().connect([this](Wt::WScrollEvent evt){
        if (!evt.viewportHeight())
        {
            return;
        }
        m_scrollPosition = (rowHeight() * getRowCount() == evt.scrollY() + evt.viewportHeight()) ? BOTTOM : NONE;
    });
}

void StreamView::addMessage(TR::StreamMessage message)
{
    addRow(std::move(message));
    m_scrollPosition = NONE;
}

StreamView::ScrollPosition StreamView::getScrollPosition() const
{
    return m_scrollPosition;
}

void StreamView::scrollBottom()
{
    scrollTo(m_model->index(m_model->getRowCount() - 1, 0));
    m_scrollPosition = BOTTOM;
}

void StreamView::addBodyColumns(const TR::XML::XmlDefNode& def_node)
{
    if (def_node.is_trivial())
    {
        addColumn(TR::XML::visit_def_node(MakeMessageBodyColumn(), def_node));
        return;
    }

    for (auto& child_def : get_child_defs(def_node))
    {
        addBodyColumns(child_def);
    }
}