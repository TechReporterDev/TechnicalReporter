#include "stdafx.h"
#include "RegularDocModel.h"
#include "Application.h"

bool RegularNodeRow::checkType(const std::type_info& tpi) const
{
    if (typeid(RegularNodeRow) == tpi)
        return true;

    return XmlNodeRow::checkType(tpi);
}

RegularDocModel* RegularNodeRow::getRegularDocModel()
{
    return static_cast<RegularDocModel*>(getXmlDocModel());
}

RegularNodeRow* RegularNodeRow::getParentRow()
{
    return static_cast<RegularNodeRow*>(XmlNodeRow::getParentRow());
}

size_t RegularNodeRow::getPosition() const
{
    return XmlNodeRow::getPosition();
}

const TR::XML::XmlRegularNode& RegularNodeRow::getRegNode() const
{
    return static_cast<const TR::XML::XmlRegularNode&>(*m_xmlNode);
}


RegularNodeRow::RegularNodeRow(TR::XML::XmlRegularNode* regNode):
    XmlNodeRow(regNode)
{
}

RegularNodeRow::RegularNodeRow(std::unique_ptr<TR::XML::XmlRegularNode> regNode):
    XmlNodeRow(std::move(regNode))
{
}

RegularNodeRow& RegularListNodeRow::addItemRow()
{
    auto model = getRegularDocModel();
    auto itemNodeRow = model->buildXmlNodeRow(getListNode().create_item());
    return static_cast<RegularNodeRow&>(addChildRow(STATIC_POINTER_CAST(XmlNodeRow, std::move(itemNodeRow))));  
}

void RegularListNodeRow::moveItemRowUp(size_t position)
{
    if (position == 0)
        return;

    swapChildRows(position, position - 1);
}

void RegularListNodeRow::moveItemRowDown(size_t position)
{
    if (position == getChildCount() - 1)
        return;

    swapChildRows(position, position + 1);
}

void RegularListNodeRow::removeItemRow(size_t position)
{
    removeChildRow(position);
}

size_t RegularListNodeRow::getItemRowCount() const
{
    return getChildCount();
}

RegularNodeRow& RegularListNodeRow::getItemRow(size_t position)
{
    return static_cast<RegularNodeRow&>(getChildRow(position));
}

const TR::XML::XmlGenericListNode& RegularListNodeRow::getListNode() const
{
    return static_cast<const TR::XML::XmlGenericListNode&>(getRegNode());
}

RegularListNodeRow::RegularListNodeRow(TR::XML::XmlGenericListNode* listNode):
    RegularNodeRow(listNode)
{   
}

RegularListNodeRow::RegularListNodeRow(std::unique_ptr<TR::XML::XmlGenericListNode> listNode):
    RegularNodeRow(std::move(listNode))
{   
}

size_t RegularStructNodeRow::getMemberRowCount() const
{
    return getChildCount();
}

RegularNodeRow& RegularStructNodeRow::getMemberRow(size_t position)
{
    return static_cast<RegularNodeRow&>(getChildRow(position));
}

const TR::XML::XmlStructNode& RegularStructNodeRow::getStructNode() const
{
    return static_cast<const TR::XML::XmlStructNode&>(getRegNode());
}

RegularStructNodeRow::RegularStructNodeRow(TR::XML::XmlStructNode* structNode):
    RegularNodeRow(structNode)
{   
}

RegularStructNodeRow::RegularStructNodeRow(std::unique_ptr<TR::XML::XmlStructNode> structNode):
    RegularNodeRow(std::move(structNode))
{   
}

std::wstring RegularTrivialNodeRow::getValue() const
{
    return getText();
}

void RegularTrivialNodeRow::setValue(const std::wstring& value)
{
    return setText(value);
}

RegularTrivialNodeRow::RegularTrivialNodeRow(TR::XML::XmlGenericTrivialNode* trivialNode):
    RegularNodeRow(trivialNode)
{
}

RegularTrivialNodeRow::RegularTrivialNodeRow(std::unique_ptr<TR::XML::XmlGenericTrivialNode> trivialNode):
    RegularNodeRow(std::move(trivialNode))
{
}

std::wstring RegularDocColumn::getString(const TR::XML::XmlNode& xmlNode) const
{
    return getString(static_cast<const TR::XML::XmlRegularNode&>(xmlNode));
}

bool RegularDocColumn::less(const TR::XML::XmlNode& left, const TR::XML::XmlNode& right) const
{
    return less(static_cast<const TR::XML::XmlRegularNode&>(left), static_cast<const TR::XML::XmlRegularNode&>(right));
}

bool RegularDocColumn::less(const TR::XML::XmlRegularNode& left, const TR::XML::XmlRegularNode& right) const
{
    return getString(left) < getString(right);
}

std::wstring RegularNameColumn::getName() const
{
    return L"Name";
}

std::wstring RegularNameColumn::getString(const TR::XML::XmlRegularNode& regNode) const
{
    if (auto structNode = TR::XML::cast_regular_node<TR::XML::XmlStructNode>(&regNode))
    {
        if (auto idAttr = structNode->find_id_attr())
        {
            return xml_tools::get_attr_wvalue(*idAttr);
        }
    }

    return regNode.get_bound_def().get_caption();
}

std::wstring RegularActionColumn::getName() const
{
    return L"Action";
}

std::wstring RegularActionColumn::getString(const TR::XML::XmlRegularNode& regNode) const
{
    return L"";
}

std::wstring RegularValueColumn::getName() const
{
    return L"Value";
}

struct GetRegNodeValue: boost::static_visitor<std::wstring>
{
    std::wstring operator()(const TR::XML::XmlRegularNode& regNode) const
    {
        return L"";
    }

    std::wstring operator()(const TR::XML::XmlGenericTrivialNode& trivialNode) const
    {
        return trivialNode.get_wtext();
    }

    std::wstring operator()(const TR::XML::XmlEnumNode& enumNode) const
    {
        auto& enumDef = enumNode.get_bound_def();
        return enumDef.get_names().at(enumNode.get_value());
    }
};

std::wstring RegularValueColumn::getString(const TR::XML::XmlRegularNode& regNode) const
{
    return visit_regular_node(GetRegNodeValue(), regNode);
}

std::wstring RegularDescriptionColumn::getName() const
{
    return L"Description";
}

std::wstring RegularDescriptionColumn::getString(const TR::XML::XmlRegularNode& regNode) const
{
    return regNode.get_bound_def().get_description();
}

struct RegularDocModel::RegularNodeRowCreator: boost::static_visitor<std::unique_ptr<RegularNodeRow>>
{
    RegularNodeRowCreator(RegularDocModel& model):
        m_model(model)
    {
    }

    std::unique_ptr<RegularNodeRow> operator()(TR::XML::XmlGenericListNode& listNode) const
    {
        return STATIC_POINTER_CAST(RegularNodeRow, m_model.createListNodeRow(&listNode));
    }

    std::unique_ptr<RegularNodeRow> operator()(TR::XML::XmlStructNode& structNode) const
    {
        return STATIC_POINTER_CAST(RegularNodeRow, m_model.createStructNodeRow(&structNode));
    }

    std::unique_ptr<RegularNodeRow> operator()(TR::XML::XmlGenericTrivialNode& trivialNode) const
    {
        return STATIC_POINTER_CAST(RegularNodeRow, m_model.createTrivialNodeRow(&trivialNode));
    }

    RegularDocModel& m_model;
};

RegularDocModel::RegularDocModel(TR::XML::XmlRegularDoc& doc)
{
    build(doc);
}

RegularDocModel::RegularDocModel()
{
}

void RegularDocModel::build(TR::XML::XmlRegularDoc& doc)
{
    XmlDocModel::build(doc);
}

void RegularDocModel::addColumn(std::unique_ptr<RegularDocColumn> column, size_t position)
{
    XmlDocModel::addColumn(std::move(column), position);
}

const RegularDocColumn& RegularDocModel::getColumn(size_t position) const
{
    return dynamic_cast<const RegularDocColumn&>(XmlDocModel::getColumn(position));
}

const TR::XML::XmlRegularDoc& RegularDocModel::getRegularDoc() const
{
    return static_cast<const TR::XML::XmlRegularDoc&>(getXmlDoc());
}

RegularNodeRow& RegularDocModel::getRootNode()
{
    return static_cast<RegularNodeRow&>(XmlDocModel::getRootNode());
}

std::unique_ptr<RegularNodeRow> RegularDocModel::buildRegularNodeRow(std::unique_ptr<TR::XML::XmlRegularNode> regNode)
{
    return STATIC_POINTER_CAST(RegularNodeRow, XmlDocModel::buildXmlNodeRow(std::move(regNode)));
}

std::unique_ptr<RegularListNodeRow> RegularDocModel::createListNodeRow(TR::XML::XmlGenericListNode* listNode)
{
    return std::unique_ptr<RegularListNodeRow>(new RegularListNodeRow(listNode));
}

std::unique_ptr<RegularStructNodeRow> RegularDocModel::createStructNodeRow(TR::XML::XmlStructNode* structNode)
{
    return std::unique_ptr<RegularStructNodeRow>(new RegularStructNodeRow(structNode));
}

std::unique_ptr<RegularTrivialNodeRow> RegularDocModel::createTrivialNodeRow(TR::XML::XmlGenericTrivialNode* trivialNode)
{
    return std::unique_ptr<RegularTrivialNodeRow>(new RegularTrivialNodeRow(trivialNode));
}

std::unique_ptr<XmlNodeRow> RegularDocModel::createXmlNodeRow(TR::XML::XmlNode* xmlNode)
{
    return STATIC_POINTER_CAST(XmlNodeRow, TR::XML::visit_regular_node(RegularNodeRowCreator(*this), static_cast<TR::XML::XmlRegularNode&>(*xmlNode)));
}