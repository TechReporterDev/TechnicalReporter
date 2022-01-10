#include "stdafx.h"
#include "XmlDocModel.h"
#include "Application.h"

XmlNodeRow::~XmlNodeRow()
{
    if (m_owner)
    {
        delete m_xmlNode;
    }
}

bool XmlNodeRow::checkType(const std::type_info& tpi) const
{
    if (typeid(XmlNodeRow) == tpi)
        return true;

    return BaseTreeModelRow::checkType(tpi);
}

XmlDocModel* XmlNodeRow::getXmlDocModel()
{
    return static_cast<XmlDocModel*>(getTreeModel());
}

void XmlNodeRow::setText(const std::wstring& text)
{
    using namespace xml_tools;
        
    auto textNodes = get_child_nodes(*m_xmlNode, xml_tools::XmlNodeType::TEXT);
    for (auto textNode = textNodes.begin(); textNode != textNodes.end(); )
    {
        xml_tools::remove_node(*textNode++);
    }
    
    if (!text.empty())
    {
        xml_tools::add_text_node(*m_xmlNode, stl_tools::ucs_to_utf8(text).c_str());
    }

    emitChanged();
}

std::wstring XmlNodeRow::getText() const
{
    return xml_tools::get_node_wtext(*m_xmlNode);
}

XmlNodeRow* XmlNodeRow::getParentRow()
{
    return static_cast<XmlNodeRow*>(BaseTreeModelRow::getParentRow());
}

size_t XmlNodeRow::getPosition() const
{
    return BaseTreeModelRow::getPosition();
}

XmlNodeRow& XmlNodeRow::addChildRow(std::unique_ptr<XmlNodeRow> xmlNodeRow, size_t position)
{
    _ASSERT(xmlNodeRow && xmlNodeRow->m_xmlNode);

    auto childNodes = get_child_nodes(*m_xmlNode, xml_tools::XmlNodeType::ELEMENT);
    auto sibling = (position == MAX_ROW_POS)? childNodes.end(): std::next(childNodes.begin(), position);
    if (sibling != childNodes.end())
    {
        xml_tools::insert_node(*sibling, std::unique_ptr<TR::XML::XmlNode>(xmlNodeRow->m_xmlNode));
    }
    else
    {
        xml_tools::add_node(*m_xmlNode, std::unique_ptr<TR::XML::XmlNode>(xmlNodeRow->m_xmlNode));
    }
    xmlNodeRow->m_owner = false;
    return static_cast<XmlNodeRow&>(BaseTreeModelRow::addChildRow(
        STATIC_POINTER_CAST(BaseTreeModelRow, xmlNodeRow), 
        position));
}


std::unique_ptr<XmlNodeRow> XmlNodeRow::removeChildRow(size_t position)
{
    auto childRow = STATIC_POINTER_CAST(XmlNodeRow, releaseChildRow(position));

    auto childNodes = get_child_nodes(*m_xmlNode, xml_tools::XmlNodeType::ELEMENT);
    auto child = childNodes.begin();
    std::advance(child, position);
    _ASSERT(child != childNodes.end());
    xml_tools::remove_node(*child).release();
    
    childRow->m_owner = true;
    return childRow;
}

size_t XmlNodeRow::getChildCount() const
{
    return BaseTreeModelRow::getChildCount();
}

XmlNodeRow& XmlNodeRow::getChildRow(size_t position)
{
    return static_cast<XmlNodeRow&>(BaseTreeModelRow::getChildRow(position));
}

void XmlNodeRow::swapChildRows(size_t first, size_t second)
{
    auto& firstRow = getChildRow(first);
    auto& secondRow = getChildRow(second);

    xml_tools::swap_nodes(*firstRow.m_xmlNode, *secondRow.m_xmlNode);
    BaseTreeModelRow::swapChildRows(first, second);
}

const TR::XML::XmlNode& XmlNodeRow::getXmlNode() const
{
    return *m_xmlNode;
}

XmlNodeRow::XmlNodeRow(TR::XML::XmlNode* xmlNode) noexcept:
    m_xmlNode(xmlNode),
    m_owner(false)
{
}

XmlNodeRow::XmlNodeRow(std::unique_ptr<TR::XML::XmlNode> xmlNode) noexcept:
    m_xmlNode(xmlNode.release()),
    m_owner(true)
{
}

std::wstring XmlDocColumn::getString(const BaseTreeModelRow& row) const
{
    return getString(static_cast<const XmlNodeRow&>(row).getXmlNode());
}

bool XmlDocColumn::less(const BaseTreeModelRow& left, const BaseTreeModelRow& right) const
{
    return less(static_cast<const XmlNodeRow&>(left).getXmlNode(), static_cast<const XmlNodeRow&>(right).getXmlNode());
}

bool XmlDocColumn::less(const TR::XML::XmlNode& left, const TR::XML::XmlNode& right) const
{
    return getString(left) < getString(right);
}

std::wstring XmlNameColumn::getName() const
{
    return L"Name";
}

std::wstring XmlNameColumn::getString(const TR::XML::XmlNode& xmlNode) const
{
    return get_node_wname(xmlNode);
}

std::wstring XmlTextColumn::getName() const
{
    return L"Text";
}

std::wstring XmlTextColumn::getString(const TR::XML::XmlNode& xmlNode) const
{
    return xml_tools::get_node_wtext(xmlNode);
}

XmlDocModel::XmlDocModel(TR::XML::XmlDoc& doc):
    m_doc(nullptr)
{
    build(doc);
}

XmlDocModel::XmlDocModel()
    :m_doc(nullptr)
{
}

void XmlDocModel::build(TR::XML::XmlDoc& doc)
{
    _ASSERT(!m_doc);
    m_doc = &doc;

    auto hiddenRoot = std::make_unique<BaseTreeModelRow>();
    hiddenRoot->addChildRow(STATIC_POINTER_CAST(BaseTreeModelRow, buildXmlNodeRow(get_root(doc))));
    reset(std::move(hiddenRoot));
}

void XmlDocModel::addColumn(std::unique_ptr<XmlDocColumn> column, size_t position)
{
    BaseTreeModel::addColumn(std::move(column), position);
}

const XmlDocColumn& XmlDocModel::getColumn(size_t position) const
{
    return static_cast<const XmlDocColumn&>(BaseTreeModel::getColumn(position));
}

const TR::XML::XmlDoc& XmlDocModel::getXmlDoc() const
{
    return *m_doc;
}

std::unique_ptr<XmlNodeRow> XmlDocModel::buildXmlNodeRow(std::unique_ptr<TR::XML::XmlNode> xmlNode)
{
    auto xmlNodeRow = buildXmlNodeRow(xmlNode.get());
    xmlNode.release();
    xmlNodeRow->m_owner = true;
    return xmlNodeRow;
}

XmlNodeRow& XmlDocModel::getRootNode()
{
    return static_cast<XmlNodeRow&>(getRow(0));
}

std::unique_ptr<XmlNodeRow> XmlDocModel::buildXmlNodeRow(TR::XML::XmlNode* xmlNode)
{
    auto xmlNodeRow = createXmlNodeRow(xmlNode);
    for (auto& childNode : get_child_nodes(*xmlNode, xml_tools::XmlNodeType::ELEMENT))
    {
        xmlNodeRow->BaseTreeModelRow::addChildRow(STATIC_POINTER_CAST(BaseTreeModelRow, buildXmlNodeRow(&childNode)));
    }
    return xmlNodeRow; 
}

std::unique_ptr<XmlNodeRow> XmlDocModel::createXmlNodeRow(TR::XML::XmlNode* xmlNode)
{
    return std::unique_ptr<XmlNodeRow>(new XmlNodeRow(xmlNode));
}