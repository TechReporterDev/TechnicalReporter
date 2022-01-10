#pragma once
#include "XMLDefinition.h"
#include "XMLRegular.h"
#include "TRXMLStd.h"
namespace TR { namespace XML {

////////////////
// Properties definition
////////////////

enum PropertyStyle {CHECK = 0, NUMBER, STRINGLINE, TEXTAREA, SELECTOR, COLLECTION, LIST};

struct XmlPropertyDef: XmlDefNode
{
    PropertyStyle get_property_style() const;
};

struct XmlCheckDef: XmlPropertyDef
{
};

struct XmlNumberDef: XmlPropertyDef
{
};

struct XmlStringLineDef: XmlPropertyDef
{
    bool is_secret() const;
};

struct XmlTextAreaDef: XmlPropertyDef
{
    int get_line_count() const;
};

struct XmlSelectorItemNode: XmlNode
{
    std::wstring get_name() const;
    std::wstring get_description() const;
};

struct XmlSelectorDef: XmlPropertyDef
{
    static const XmlSelectorItemNode* next_item(const XmlSelectorItemNode* item_node);
    using ConstItemIterator = xml_tools::node_iterator<const XmlSelectorItemNode, next_item>;
    using ConstItemRange = boost::iterator_range<ConstItemIterator>;

    ConstItemIterator   begin_item() const;
    ConstItemIterator   end_item() const;
    ConstItemRange      get_items() const;
};

struct XmlPropertyCollectionDef: XmlPropertyDef
{
    static const XmlPropertyDef* next_member_def(const XmlPropertyDef* property_def);
    using ConstMemberDefIterator = xml_tools::node_iterator<const XmlPropertyDef, next_member_def>;
    using ConstMemberDefRange = boost::iterator_range<ConstMemberDefIterator>;
    
    ConstMemberDefIterator      begin_member_def() const;
    ConstMemberDefIterator      end_member_def() const;
    ConstMemberDefRange         get_members_def() const;
};

struct XmlPropertyListDef: XmlPropertyDef
{
    const XmlPropertyCollectionDef& get_item_def() const;
};

struct XmlPropertiesDef: XmlDefDoc
{
    const XmlPropertyCollectionDef& get_root_def() const;
};

// navigation functions
template<class Visitor> 
typename Visitor::result_type visit_property_def(Visitor visitor, XmlPropertyDef& property_def);

////////////////
// Properties document
////////////////

struct XmlPropertyNode: XmlRegularNode
{
    const XmlPropertyDef& get_property_def() const;
};

struct XmlCheckNode: XmlPropertyNode
{
    const XmlCheckDef&      get_property_def() const;
    XmlBooleanNode&         get_value_node();
    const XmlBooleanNode&   get_value_node() const; 
    
    bool                    is_checked() const;
    void                    set_checked(bool checked);
};

struct XmlNumberNode: XmlPropertyNode
{
    const XmlNumberDef&     get_property_def() const;
    XmlIntegerNode&         get_value_node();
    const XmlIntegerNode&   get_value_node() const; 

    int                     get_number() const;
    void                    set_number(int number);
};

struct XmlStringLineNode: XmlPropertyNode
{
    const XmlStringLineDef& get_property_def() const;
    XmlStringNode&          get_value_node();
    const XmlStringNode&    get_value_node() const; 

    std::wstring            get_string() const;
    void                    set_string(const std::wstring& string);
};

struct XmlTextAreaNode: XmlPropertyNode
{
    const XmlTextAreaDef&   get_property_def() const;
    XmlStringNode&          get_value_node();
    const XmlStringNode&    get_value_node() const; 

    std::wstring            get_text() const;
    void                    set_text(const std::wstring& text);
};

struct XmlSelectorNode: XmlPropertyNode
{
    const XmlSelectorDef&   get_property_def() const;
    XmlIntegerNode&         get_value_node();
    const XmlIntegerNode&   get_value_node() const; 

    int                     get_selection() const;
    void                    set_selection(int selection);
};

struct XmlPropertyCollectionNode: XmlPropertyNode
{   
    using PropertyIterator = boost::transform_iterator<std::function<XmlPropertyNode&(XmlRegularNode&)>, XmlStructNode::MemberIterator>;
    using PropertyRange = boost::iterator_range<PropertyIterator>;

    const XmlPropertyCollectionDef&     get_property_def() const;

    PropertyIterator                    begin_property();
    PropertyIterator                    end_property();
    PropertyRange                       get_properties();
};

struct XmlPropertyListNode: XmlPropertyNode
{
    using PropertyIterator = boost::transform_iterator<std::function<XmlPropertyCollectionNode&(XmlRegularNode&)>, XmlStructListNode::ItemIterator>;
    using PropertyRange = boost::iterator_range<PropertyIterator>;

    const XmlPropertyListDef&                       get_property_def() const;

    std::unique_ptr<XmlPropertyCollectionNode>      create_property() const;
    PropertyIterator                                insert_property(PropertyIterator _where, std::unique_ptr<XmlPropertyCollectionNode> collection_node = nullptr);
    void                                            remove_property(PropertyIterator _where);   
    
    PropertyIterator                                begin_property();
    PropertyIterator                                end_property();
    PropertyRange                                   get_properties();
};

struct XmlPropertiesDoc: XmlRegularDoc
{
    const XmlPropertiesDef&             get_properties_def() const;

    XmlPropertyCollectionNode&          get_root();
    const XmlPropertyCollectionNode&    get_root() const;
};

// create functions
std::unique_ptr<XmlPropertiesDef>   parse_properties_def(const std::string& xml_properties_def);
std::unique_ptr<XmlPropertiesDoc>   parse_properties_doc(const std::string& xml_properties_doc, const XmlPropertiesDef& properties_def);
std::unique_ptr<XmlPropertiesDoc>   create_properties_doc(const XmlPropertiesDef& properties_def);

// navigation functions
template<class Visitor> 
typename Visitor::result_type visit_property_node(Visitor visitor, XmlPropertyNode& property_node);

///////////////
// Implementation
///////////////

template<class Visitor> 
typename Visitor::result_type visit_property_def(Visitor visitor, const XmlPropertyDef& property_def)
{
    switch (property_def.get_property_style())
    {
    case CHECK:
        return visitor(static_cast<const XmlCheckDef&>(property_def));

    case NUMBER:
        return visitor(static_cast<const XmlNumberDef&>(property_def));

    case STRINGLINE:
        return visitor(static_cast<const XmlStringLineDef&>(property_def));

    case TEXTAREA:
        return visitor(static_cast<const XmlTextAreaDef&>(property_def));

    case SELECTOR:
        return visitor(static_cast<const XmlSelectorDef&>(property_def));

    case COLLECTION:
        return visitor(static_cast<const XmlPropertyCollectionDef&>(property_def));

    case LIST:
        return visitor(static_cast<const XmlPropertyListDef&>(property_def));

    default:
        _ASSERT(false);
    }   
    throw std::logic_error("Invalid property type found");  
}

template<class Visitor> 
typename Visitor::result_type visit_property_node(Visitor visitor, XmlPropertyNode& property_node)
{
    auto& property_def = property_node.get_property_def();
    switch (property_def.get_property_style())
    {
    case CHECK:
        return visitor(static_cast<XmlCheckNode&>(property_node));

    case NUMBER:
        return visitor(static_cast<XmlNumberNode&>(property_node));

    case STRINGLINE:
        return visitor(static_cast<XmlStringLineNode&>(property_node));

    case TEXTAREA:
        return visitor(static_cast<XmlTextAreaNode&>(property_node));

    case SELECTOR:
        return visitor(static_cast<XmlSelectorNode&>(property_node));

    case COLLECTION:
        return visitor(static_cast<XmlPropertyCollectionNode&>(property_node));

    case LIST:
        return visitor(static_cast<XmlPropertyListNode&>(property_node));

    default:
        _ASSERT(false);
    }
    throw std::logic_error("Invalid property type found");  
}

}} //namespace TR { namespace XML {