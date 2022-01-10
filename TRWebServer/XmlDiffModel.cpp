#include "stdafx.h"
#include "XmlDiffModel.h"
#include "Application.h"

XmlDiffNodeRow::XmlDiffNodeRow(const TR::XML::XmlNode* newNode, const TR::XML::XmlNode* oldNode, State state):
    m_newNode(newNode),
    m_oldNode(oldNode),
    m_state(state)
{
}

XmlDiffModel* XmlDiffNodeRow::getXmlDiffModel() const
{
    return static_cast<XmlDiffModel*>(getTreeModel());
}

const TR::XML::XmlNode* XmlDiffNodeRow::getNewNode() const
{
    return m_newNode;
}

const TR::XML::XmlNode* XmlDiffNodeRow::getOldNode() const
{
    return m_oldNode;
}

XmlDiffNodeRow::State XmlDiffNodeRow::getDiffState() const
{
    return m_state;
}

bool XmlDiffNodeRow::checkType(const std::type_info& tpi) const
{
    if (typeid(XmlDiffNodeRow) == tpi)
    {
        return true;
    }
    return BaseTreeModelRow::checkType(tpi);
}

XmlDiffNodeRow& XmlDiffNodeRow::addChildRow(std::unique_ptr<XmlDiffNodeRow> diffNodeRow, size_t position)
{
    return static_cast<XmlDiffNodeRow&>(BaseTreeModelRow::addChildRow(std::unique_ptr<BaseTreeModelRow>(diffNodeRow.release()), position));
}

std::wstring XmlDiffColumn::getString(const BaseTreeModelRow& row) const
{
    return getString(static_cast<const XmlDiffNodeRow&>(row));
}

bool XmlDiffColumn::less(const BaseTreeModelRow& left, const BaseTreeModelRow& right) const
{
    return less(static_cast<const XmlDiffNodeRow&>(left), static_cast<const XmlDiffNodeRow&>(right));
}

bool XmlDiffColumn::less(const XmlDiffNodeRow& left, const XmlDiffNodeRow& right) const
{
    return BaseTreeModelColumn::less(left, right);
}

std::wstring XmlDiffNameColumn::getName() const
{
    return L"Name";
}

std::wstring XmlDiffNameColumn::getString(const XmlDiffNodeRow& diffRow) const
{
    auto node = diffRow.getNewNode();
    if (!node)
    {
        node = diffRow.getOldNode();
    }

    _ASSERT(node);
    return get_node_wname(*node);
}

std::wstring XmlDiffNewValueColumn::getName() const
{
    return L"New value";
}

std::wstring XmlDiffNewValueColumn::getString(const XmlDiffNodeRow& diffRow) const
{
    if (auto newNode = diffRow.getNewNode())
    {
        return xml_tools::get_node_wtext(*newNode);
    }
    return L"";
}

std::wstring XmlDiffOldValueColumn::getName() const
{
    return L"Old value";
}

std::wstring XmlDiffOldValueColumn::getString(const XmlDiffNodeRow& diffRow) const
{
    if (auto oldNode = diffRow.getOldNode())
    {
        return xml_tools::get_node_wtext(*oldNode);
    }
    return L"";
}

XmlDiffModel::XmlDiffModel(const TR::XML::XmlDoc& newDoc, const TR::XML::XmlDoc& oldDoc, const std::vector<TR::Core::XmlDiffPoint>& diffPoints):
    m_newDoc(nullptr),
    m_oldDoc(nullptr)
{
    build(newDoc, oldDoc, diffPoints);
}

XmlDiffModel::XmlDiffModel():
    m_newDoc(nullptr),
    m_oldDoc(nullptr)
{
}

void XmlDiffModel::build(const TR::XML::XmlDoc& newDoc, const TR::XML::XmlDoc& oldDoc, const std::vector<TR::Core::XmlDiffPoint>& diffPoints)
{
    _ASSERT(!m_newDoc && !m_oldDoc);
    m_newDoc = &newDoc;
    m_oldDoc = &oldDoc;
    
    auto hiddenRoot = std::make_unique<XmlDiffNodeRow>(nullptr);
    hiddenRoot->addChildRow(buildDiffNodeRow(get_root(newDoc)));
    reset(std::unique_ptr<BaseTreeModelRow>(hiddenRoot.release()));

    for (auto& diffPoint : diffPoints)
    {
        addDiffPoint(diffPoint);
    }
}

void XmlDiffModel::addColumn(std::unique_ptr<XmlDiffColumn> column)
{
    BaseTreeModel::addColumn(std::move(column));
}

XmlDiffColumn& XmlDiffModel::getColumn(size_t position) const
{
    return dynamic_cast<XmlDiffColumn&>(BaseTreeModel::getColumn(position));
}

std::unique_ptr<XmlDiffNodeRow> XmlDiffModel::createDiffNodeRow(const TR::XML::XmlNode* newNode, const TR::XML::XmlNode* oldNode)
{
    _ASSERT((newNode || oldNode) && (!newNode || !oldNode));
    return std::make_unique<XmlDiffNodeRow>(newNode, oldNode);
}
    
std::unique_ptr<XmlDiffNodeRow> XmlDiffModel::buildDiffNodeRow(const TR::XML::XmlNode* newNode, const TR::XML::XmlNode* oldNode)
{
    _ASSERT((newNode || oldNode) && (!newNode || !oldNode));
    auto diffNodeRow = createDiffNodeRow(newNode, oldNode);

    if (newNode)
    {
        for (auto& child : get_child_nodes(*newNode, xml_tools::XmlNodeType::ELEMENT))
        {
            diffNodeRow->addChildRow(buildDiffNodeRow(&child, nullptr));
        }
    }

    if (oldNode)
    {
        for (auto& child : get_child_nodes(*oldNode, xml_tools::XmlNodeType::ELEMENT))
        {
            diffNodeRow->addChildRow(buildDiffNodeRow(nullptr, &child));
        }
    }

    return diffNodeRow;
}

void XmlDiffModel::updateState(XmlDiffNodeRow& diffNodeRow, XmlDiffNodeRow::State state)
{
    diffNodeRow.m_state = state;
    if (auto parentRow = static_cast<XmlDiffNodeRow*>(diffNodeRow.getParentRow()))
    {
        if (parentRow->m_state == XmlDiffNodeRow::STATE_NONE)
        {
            updateState(*parentRow, XmlDiffNodeRow::STATE_MODIFIED);
        }
    }

    if (state == XmlDiffNodeRow::STATE_INSERTED || state == XmlDiffNodeRow::STATE_REMOVED)
    {
        for (size_t position = 0; position < diffNodeRow.getChildCount(); ++position)
        {
            auto& child_row = static_cast<XmlDiffNodeRow&>(diffNodeRow.getChildRow(position));
            updateState(child_row, state);
        }
    }

}

void XmlDiffModel::addDiffPoint(const TR::Core::XmlDiffPoint& diffPoint)
{
    auto first = diffPoint.m_path.begin();
    auto last = diffPoint.m_path.end(); 
    _ASSERT(first != last);

    switch (diffPoint.get_type())
    {
    case TR::Core::XmlDiffPoint::ELEMENT:
        if (diffPoint.m_state == TR::Core::XmlDiffPoint::STATE_INSERTED)
        {
            auto& diffRow = getDiffRow(first, last);
            updateState(diffRow, XmlDiffNodeRow::STATE_INSERTED);
        }
        else
        {
            _ASSERT(diffPoint.m_state == TR::Core::XmlDiffPoint::STATE_REMOVED);
            
            auto& parentRow = getDiffRow(first, last - 1);
            auto oldNode = &getOldNode(first, last);
            auto& diffRow = parentRow.addChildRow(buildDiffNodeRow(nullptr, oldNode), (last - 1)->m_position);
            updateState(diffRow, XmlDiffNodeRow::STATE_REMOVED);
        }
        break;

    case TR::Core::XmlDiffPoint::TEXT:
        if (diffPoint.m_state == TR::Core::XmlDiffPoint::STATE_REMOVED)
        {
            auto& diffRow = getDiffRow(first, last - 1);
            diffRow.m_oldNode = &getOldNode(first, last - 1);
            updateState(diffRow, XmlDiffNodeRow::STATE_MODIFIED);
        }
        break;

    case TR::Core::XmlDiffPoint::ATTRIBUTE:
        break;

    case TR::Core::XmlDiffPoint::ATTR_VALUE:
        break;

    default:
        _ASSERT(false);
    }
}

XmlDiffNodeRow& XmlDiffModel::getDiffRow(TR::Core::XmlDiffPoint::Path::const_iterator first, TR::Core::XmlDiffPoint::Path::const_iterator last)
{
    auto current = &getHiddenRoot();
    for (auto i = first; i != last; ++i)
    {
        current = &current->getChildRow(i->m_position);
    }
    return static_cast<XmlDiffNodeRow&>(*current);
}

static const TR::XML::XmlNode& getXmlNode(const TR::XML::XmlNode& parentNode, const std::string& name)
{
    if (auto child = xml_tools::find_child(parentNode, name.c_str()))
    {
        return *child;
    }

    throw std::logic_error("Invalid XML document format");
}

static const TR::XML::XmlNode& getXmlNode(const TR::XML::XmlNode& parentNode, const std::string& name, const std::string& id)
{
    using namespace xml_tools;
    for (auto& child : get_child_nodes(parentNode, [&](const XmlNode& node){ return node_name_equal(node, name.c_str()); }))
    {
        if (auto idAttr = find_attribute(child, "id"))
        {
            if (attr_value_equal(*idAttr, id.c_str()))
            {
                return child;
            }
        }
    }

    throw std::logic_error("Invalid regular document format");
}

const TR::XML::XmlNode& XmlDiffModel::getOldNode(TR::Core::XmlDiffPoint::Path::const_iterator first, TR::Core::XmlDiffPoint::Path::const_iterator last) const
{
    const TR::XML::XmlNode* current = nullptr;
    for (auto i = first; i != last; ++i)
    {
        if (!current)
        {
            current = get_root(*m_oldDoc);
            _ASSERT(node_name_equal(*current, i->m_name.c_str()));
            continue;
        }

        if (i->m_id)
        {
            current = &getXmlNode(*current, i->m_name, *i->m_id);
        }
        else
        {
            current = &getXmlNode(*current, i->m_name);
        }       
    }
    return *current;
}