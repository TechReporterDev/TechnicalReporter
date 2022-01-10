#include "stdafx.h"
#include "XMLProperties.h"
namespace TR { namespace XML {
using namespace xml_tools;
static const int DEFAULT_LINE_COUNT = 10;
static const std::wstring DEFAULT_ITEM_NAME = L"default";
static const std::wstring DEFAULT_ITEM_DESCRIPTION = L"";

static PropertyStyle get_property_style(const XmlNode& node)
{
    auto style_attr = find_attribute(node, "style");
    if (!style_attr)
    {
        return COLLECTION;
    }

    if (attr_value_equal(*style_attr, "CHECK"))
    {
        return CHECK;
    }
    else if (attr_value_equal(*style_attr, "NUMBER"))
    {
        return NUMBER;
    }
    else if (attr_value_equal(*style_attr, "STRINGLINE"))
    {
        return STRINGLINE;
    }
    else if (attr_value_equal(*style_attr, "TEXTAREA"))
    {
        return TEXTAREA;
    }
    else if (attr_value_equal(*style_attr, "SELECTOR"))
    {
        return SELECTOR;
    }
    else if (attr_value_equal(*style_attr, "COLLECTION"))
    {
        return COLLECTION;
    }
    else if (attr_value_equal(*style_attr, "LIST"))
    {
        return LIST;
    }
    else
    {
        throw std::logic_error("Invalid property style found");
    }
}

static const XmlMetaNode& get_property_meta(const XmlPropertyDef& property_def)
{   
    auto prop_meta = property_def.begin_meta_node();
    _ASSERT(prop_meta != property_def.end_meta_node() && node_name_equal(*prop_meta, "property"));
    return *prop_meta;
}

static XmlMetaNode& get_property_meta(XmlPropertyDef& property_def)
{
    auto prop_meta = property_def.begin_meta_node();
    _ASSERT(prop_meta != property_def.end_meta_node() && node_name_equal(*prop_meta, "property"));
    return const_cast<XmlMetaNode&>(*prop_meta);
}

PropertyStyle XmlPropertyDef::get_property_style() const
{
    return XML::get_property_style(get_property_meta(*this));   
}

bool XmlStringLineDef::is_secret() const
{
    if (auto secret_attr = find_attribute(get_property_meta(*this), "secret"))
    {
        return attr_value_equal(*secret_attr, "1");
    }
    return false;
}

int XmlTextAreaDef::get_line_count() const
{
    if (auto line_count_attr = find_attribute(get_property_meta(*this), "line_count"))
    {
        return boost::lexical_cast<int>(get_attr_wvalue(*line_count_attr));
    }
    return DEFAULT_LINE_COUNT;
}

std::wstring XmlSelectorItemNode::get_name() const
{
    return get_attr_wvalue(*this, "name", DEFAULT_ITEM_NAME.c_str());
}

std::wstring XmlSelectorItemNode::get_description() const
{
    return get_attr_wvalue(*this, "description", DEFAULT_ITEM_DESCRIPTION.c_str());
}

const XmlSelectorItemNode* XmlSelectorDef::next_item(const XmlSelectorItemNode* item_node)
{
    auto next_item_node = next_child(item_node);
    return static_cast<const XmlSelectorItemNode*>(next_item_node);
}

XmlSelectorDef::ConstItemIterator XmlSelectorDef::begin_item() const
{
    auto item_node = begin_child(get_property_meta(*this));
    _ASSERT(item_node != nullptr);
    return ConstItemIterator(static_cast<const XmlSelectorItemNode*>(item_node));
}

XmlSelectorDef::ConstItemIterator XmlSelectorDef::end_item() const
{
    return ConstItemIterator();
}

XmlSelectorDef::ConstItemRange XmlSelectorDef::get_items() const
{
    return ConstItemRange(begin_item(), end_item());
}

const XmlPropertyDef* XmlPropertyCollectionDef::next_member_def(const XmlPropertyDef* property_def)
{
    _ASSERT(property_def);
    return static_cast<const XmlPropertyDef*>(XmlStructDef::next_member_def(property_def));
}

XmlPropertyCollectionDef::ConstMemberDefIterator XmlPropertyCollectionDef::begin_member_def() const
{
    auto& struct_def = static_cast<const XmlStructDef&>(static_cast<const XmlDefNode&>(*this));
    return ConstMemberDefIterator(static_cast<const XmlPropertyDef*>(struct_def.begin_member_def().get_pos()));
}

XmlPropertyCollectionDef::ConstMemberDefIterator XmlPropertyCollectionDef::end_member_def() const
{
    return ConstMemberDefIterator();
}

XmlPropertyCollectionDef::ConstMemberDefRange XmlPropertyCollectionDef::get_members_def() const
{
    return ConstMemberDefRange(begin_member_def(), end_member_def());
}

const XmlPropertyCollectionDef& XmlPropertyListDef::get_item_def() const
{
    auto& list_def = static_cast<const XmlListDef&>(static_cast<const XmlDefNode&>(*this)); 
    return static_cast<const XmlPropertyCollectionDef&>(list_def.get_item_def());
}

const XmlPropertyCollectionDef& XmlPropertiesDef::get_root_def() const
{
    return static_cast<const XmlPropertyCollectionDef&>(*XmlDefDoc::get_root_def());
}

const XmlCheckDef& XmlCheckNode::get_property_def() const
{
    return static_cast<const XmlCheckDef&>(get_bound_def());
}

XmlBooleanNode& XmlCheckNode::get_value_node()
{
    return static_cast<XmlBooleanNode&>(static_cast<XmlRegularNode&>(*this));   
}

const XmlBooleanNode& XmlCheckNode::get_value_node() const
{
    return const_cast<XmlCheckNode&>(*this).get_value_node();
}

bool XmlCheckNode::is_checked() const
{
    return get_value_node().get_value();
}

void XmlCheckNode::set_checked(bool checked)
{   
    get_value_node().set_value(checked);
}

const XmlNumberDef& XmlNumberNode::get_property_def() const
{
    return static_cast<const XmlNumberDef&>(get_bound_def());
}

XmlIntegerNode& XmlNumberNode::get_value_node()
{
    return static_cast<XmlIntegerNode&>(static_cast<XmlRegularNode&>(*this));
}

const XmlIntegerNode& XmlNumberNode::get_value_node() const
{
    return const_cast<XmlNumberNode&>(*this).get_value_node();
}

int XmlNumberNode::get_number() const
{
    return get_value_node().get_value();
}

void XmlNumberNode::set_number(int number)
{
    get_value_node().set_value(number);
};

const XmlStringLineDef& XmlStringLineNode::get_property_def() const
{
    return static_cast<const XmlStringLineDef&>(get_bound_def());
}

XmlStringNode& XmlStringLineNode::get_value_node()
{
    return static_cast<XmlStringNode&>(static_cast<XmlRegularNode&>(*this));
}

const XmlStringNode& XmlStringLineNode::get_value_node() const
{
    return const_cast<XmlStringLineNode&>(*this).get_value_node();
}

std::wstring XmlStringLineNode::get_string() const
{
    return get_value_node().get_value();
}

void XmlStringLineNode::set_string(const std::wstring& string)
{
    get_value_node().set_value(string);
}

const XmlTextAreaDef& XmlTextAreaNode::get_property_def() const
{
    return static_cast<const XmlTextAreaDef&>(get_bound_def());
}

XmlStringNode& XmlTextAreaNode::get_value_node()
{
    return static_cast<XmlStringNode&>(static_cast<XmlRegularNode&>(*this));
}

const XmlStringNode& XmlTextAreaNode::get_value_node() const
{
    return const_cast<XmlTextAreaNode&>(*this).get_value_node();
}

std::wstring XmlTextAreaNode::get_text() const
{
    return get_value_node().get_value();
}

void XmlTextAreaNode::set_text(const std::wstring& text)
{
    get_value_node().set_value(text);
}

const XmlSelectorDef& XmlSelectorNode::get_property_def() const
{
    return static_cast<const XmlSelectorDef&>(get_bound_def());
}

XmlIntegerNode& XmlSelectorNode::get_value_node()
{
    return static_cast<XmlIntegerNode&>(static_cast<XmlRegularNode&>(*this));
}

const XmlIntegerNode& XmlSelectorNode::get_value_node() const
{
    return const_cast<XmlSelectorNode&>(*this).get_value_node();
}

int XmlSelectorNode::get_selection() const
{
    return get_value_node().get_value();
}

void XmlSelectorNode::set_selection(int selection)
{
    get_value_node().set_value(selection);
};

const XmlPropertyCollectionDef& XmlPropertyCollectionNode::get_property_def() const
{
    return static_cast<const XmlPropertyCollectionDef&>(XmlRegularNode::get_bound_def());
}

XmlStructNode& get_struct_node(XmlPropertyCollectionNode& property_collection_node)
{
    return static_cast<XmlStructNode&>(static_cast<XmlRegularNode&>(property_collection_node));
}

const XmlStructNode& get_struct_node(const XmlPropertyCollectionNode& property_collection_node)
{
    return get_struct_node(const_cast<XmlPropertyCollectionNode&>(property_collection_node));
}

XmlPropertyCollectionNode::PropertyIterator XmlPropertyCollectionNode::begin_property()
{
    return PropertyIterator(get_struct_node(*this).begin_member(), [](XmlRegularNode& reg_node) -> XmlPropertyNode& {
        return static_cast<XmlPropertyNode&>(reg_node); 
    });
}

XmlPropertyCollectionNode::PropertyIterator XmlPropertyCollectionNode::end_property()
{
    return PropertyIterator(get_struct_node(*this).end_member(), nullptr);
}

XmlPropertyCollectionNode::PropertyRange XmlPropertyCollectionNode::get_properties()
{
    return PropertyRange(begin_property(), end_property());
}

const XmlPropertyListDef& XmlPropertyListNode::get_property_def() const
{
    return static_cast<const XmlPropertyListDef&>(XmlRegularNode::get_bound_def());
}

XmlStructListNode& get_list_node(XmlPropertyListNode& property_list_node)
{
    return static_cast<XmlStructListNode&>(static_cast<XmlRegularNode&>(property_list_node));
}

const XmlStructListNode& get_list_node(const XmlPropertyListNode& property_list_node)
{
    return get_list_node(const_cast<XmlPropertyListNode&>(property_list_node));
}

std::unique_ptr<XmlPropertyCollectionNode> XmlPropertyListNode::create_property() const
{
    return stl_tools::static_pointer_cast<XmlPropertyCollectionNode>(stl_tools::static_pointer_cast<XmlRegularNode>(get_list_node(*this).create_item()));
}

XmlPropertyListNode::PropertyIterator XmlPropertyListNode::insert_property(PropertyIterator _where, std::unique_ptr<XmlPropertyCollectionNode> collection_node)
{
    auto position = get_list_node(*this).insert_item(
        _where.base(), 
        stl_tools::static_pointer_cast<XmlStructNode>(stl_tools::static_pointer_cast<XmlRegularNode>(std::move(collection_node))));

    return PropertyIterator(position, [](XmlRegularNode& reg_node) -> XmlPropertyCollectionNode& {
        return static_cast<XmlPropertyCollectionNode&>(reg_node);
    });
}

void XmlPropertyListNode::remove_property(PropertyIterator _where)
{
    get_list_node(*this).remove_item(_where.base());
}

XmlPropertyListNode::PropertyIterator XmlPropertyListNode::begin_property()
{
    return PropertyIterator(get_list_node(*this).begin_item(), [](XmlRegularNode& reg_node) -> XmlPropertyCollectionNode& {
        return static_cast<XmlPropertyCollectionNode&>(reg_node);
    });
}

XmlPropertyListNode::PropertyIterator XmlPropertyListNode::end_property()
{
    return PropertyIterator(get_list_node(*this).end_item(), nullptr);
}

XmlPropertyListNode::PropertyRange XmlPropertyListNode::get_properties()
{
    return PropertyRange(begin_property(), end_property());
}

const XmlPropertyDef& XmlPropertyNode::get_property_def() const
{
    return static_cast<const XmlPropertyDef&>(XmlRegularNode::get_bound_def());
}

const XmlPropertiesDef& XmlPropertiesDoc::get_properties_def() const
{
    return static_cast<const XmlPropertiesDef&>(XmlRegularDoc::get_bound_def());
}

XmlPropertyCollectionNode& XmlPropertiesDoc::get_root()
{
    return static_cast<XmlPropertyCollectionNode&>(*XmlRegularDoc::get_root());
}

const XmlPropertyCollectionNode& XmlPropertiesDoc::get_root() const
{
    return static_cast<const XmlPropertyCollectionNode&>(*XmlRegularDoc::get_root());
}

static std::unique_ptr<XmlDefNode> create_def_node(const std::string& name, const std::string& class_)
{
    auto property_def = create_element(name.c_str());
    set_attr_value(*property_def, "class", class_.c_str());
    return stl_tools::static_pointer_cast<XmlDefNode>(std::move(property_def));
}

static XmlMetaNode& add_property_meta(XmlDefNode& def_node, const std::string& style)
{
    auto& meta_node = add_element(def_node, "META");
    auto& prop_meta = add_element(meta_node, "property");
    set_attr_value(prop_meta, "style", style.c_str());
    return static_cast<XmlMetaNode&>(prop_meta);
}

static void copy_attributes(XmlNode& where_, const XmlNode& from, std::initializer_list<const char*> names)
{
    for (const char* name : names)
    {
        if (auto attr = find_attribute(from, name))
        {
            set_attr_value(where_, name, get_attr_value(*attr).c_str());
        }
    }
}

static void copy_items(XmlMetaNode& property_meta, const XmlNode& xml_node)
{
    for (auto& item_node : get_child_nodes(xml_node))
    {
        add_node(property_meta, clone_node(item_node, XmlCloneOpt::ATTRIBUTE));
    }
}

static std::unique_ptr<XmlCheckDef> create_check_def(const XmlNode& xml_node)
{
    auto check_def = create_def_node(get_node_name(xml_node), "BOOLEAN");
    copy_attributes(*check_def, xml_node, {"caption", "description"});

    auto& check_meta = add_property_meta(*check_def, "CHECK");  
    copy_attributes(check_meta, xml_node, {"read_only"});
    return stl_tools::static_pointer_cast<XmlCheckDef>(std::move(check_def));
}

static std::unique_ptr<XmlNumberDef> create_number_def(const XmlNode& xml_node)
{
    auto number_def = create_def_node(get_node_name(xml_node), "INTEGER");
    copy_attributes(*number_def, xml_node, {"caption", "description"});

    auto& number_meta = add_property_meta(*number_def, "NUMBER");
    copy_attributes(number_meta, xml_node, {"read_only"});  
    return stl_tools::static_pointer_cast<XmlNumberDef>(std::move(number_def));
}

static std::unique_ptr<XmlStringLineDef> create_string_line_def(const XmlNode& xml_node)
{
    auto string_def = create_def_node(get_node_name(xml_node), "STRING");
    copy_attributes(*string_def, xml_node, {"caption", "description"});

    auto& string_meta = add_property_meta(*string_def, "STRINGLINE");
    copy_attributes(string_meta, xml_node, {"read_only", "secret"});
    return stl_tools::static_pointer_cast<XmlStringLineDef>(std::move(string_def));
}

static std::unique_ptr<XmlTextAreaDef> create_text_area_def(const XmlNode& xml_node)
{
    auto text_area_def = create_def_node(get_node_name(xml_node), "STRING");
    copy_attributes(*text_area_def, xml_node, {"caption", "description"});

    auto& text_area_meta = add_property_meta(*text_area_def, "TEXTAREA");
    copy_attributes(text_area_meta, xml_node, {"read_only", "line_count"});
    return stl_tools::static_pointer_cast<XmlTextAreaDef>(std::move(text_area_def));
}

static std::unique_ptr<XmlSelectorDef> create_selector_def(const XmlNode& xml_node)
{
    auto selector_def = create_def_node(get_node_name(xml_node), "INTEGER");
    copy_attributes(*selector_def, xml_node, {"caption", "description"});

    auto& selector_meta = add_property_meta(*selector_def, "SELECTOR");
    copy_attributes(selector_meta, xml_node, {"read_only"});
    copy_items(selector_meta, xml_node);    
    return stl_tools::static_pointer_cast<XmlSelectorDef>(std::move(selector_def));
}

static std::unique_ptr<XmlPropertyDef> create_property_def(const XmlNode& xml_node);
static std::unique_ptr<XmlPropertyCollectionDef> create_collection_def(const XmlNode& xml_node)
{
    auto collection_def = create_def_node(get_node_name(xml_node), "STRUCT");
    copy_attributes(*collection_def, xml_node, {"caption", "description"});
    add_property_meta(*collection_def, "COLLECTION");

    for (auto& child_node : get_child_nodes(xml_node))
    {
        add_node(*collection_def, create_property_def(child_node));
    }
    return stl_tools::static_pointer_cast<XmlPropertyCollectionDef>(std::move(collection_def));
}

static std::unique_ptr<XmlPropertyListDef> create_list_def(const XmlNode& xml_node)
{
    auto list_def = create_def_node(get_node_name(xml_node), "LIST");
    copy_attributes(*list_def, xml_node, {"caption", "description"});
    add_property_meta(*list_def, "LIST");

    auto item_node = begin_child(xml_node);
    if (item_node == nullptr)
    {
        throw std::logic_error("Invalid list definition");
    }
    add_node(*list_def, create_collection_def(*item_node));
    return stl_tools::static_pointer_cast<XmlPropertyListDef>(std::move(list_def));
}

static std::unique_ptr<XmlPropertyDef> create_property_def(const XmlNode& xml_node)
{
    switch (get_property_style(xml_node))
    {
    case CHECK:
        return create_check_def(xml_node);

    case NUMBER:
        return create_number_def(xml_node);

    case STRINGLINE:
        return create_string_line_def(xml_node);

    case TEXTAREA:
        return create_text_area_def(xml_node);

    case SELECTOR:
        return create_selector_def(xml_node);

    case COLLECTION:
        return create_collection_def(xml_node);

    case LIST:
        return create_list_def(xml_node);

    default:
        _ASSERT(false);
    }
    throw std::logic_error("Invalid property type found");
}

std::unique_ptr<XmlPropertiesDef> parse_properties_def(const std::string& xml_properties_def)
{
    auto def_doc = stl_tools::static_pointer_cast<XmlPropertiesDef>(create_doc());
    auto xml_doc = parse(xml_properties_def);
    auto root = get_root(*xml_doc);
    auto& def_root = set_root(*def_doc, create_property_def(*root));
    return def_doc;
}

std::unique_ptr<XmlPropertiesDoc> parse_properties_doc(const std::string& xml_properties_doc, const XmlPropertiesDef& properties_def)
{
    return stl_tools::static_pointer_cast<XmlPropertiesDoc>(parse_regular(xml_properties_doc, properties_def));
}

std::unique_ptr<XmlPropertiesDoc> create_properties_doc(const XmlPropertiesDef& properties_def)
{
    return stl_tools::static_pointer_cast<XmlPropertiesDoc>(create_regular(properties_def));
}

}} //namespace TR { namespace XML {