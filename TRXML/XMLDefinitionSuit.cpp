#include "stdafx.h"
#include "XMLDefinitionSuit.h"
#include "XMLDefinition.h"
#include "XMLRegular.h"
#include "XmlTools\xml_tools.h"
using namespace TR::XML;
using namespace xml_tools;

namespace {

void read_definition_test()
{
    std::string xml_def(
        "<?xml version=\"1.0\"?>"
        "<root class=\"STRUCT\">"
            "<list class=\"LIST\" caption=\"N_LIST\" description=\"D_LIST\">"               
            "</list>"
            "<struct class=\"STRUCT\" caption=\"N_STRUCT\" description=\"D_STRUCT\">"
            "</struct>"
            "<string class=\"STRING\" caption=\"N_STRING\" description=\"D_STRING\">"
            "</string>"
            "<blah class=\"BLAH\">"
            "</blah>"
        "</root>");

    auto def_doc = stl_tools::static_pointer_cast<XmlDefDoc>(parse(xml_def)); //not parse_definition to avoid checking
    auto def_root = cast_def_node<XmlStructDef>(def_doc->get_root_def());
    auto def_childs = def_root->begin_member_def();

    auto& list_def = *def_childs++;
    TEST_ASSERT(list_def.get_class() == XmlNodeClass::LIST);
    TEST_ASSERT(list_def.get_caption() == L"N_LIST");
    TEST_ASSERT(list_def.get_description() == L"D_LIST");

    auto& struct_def = *def_childs++;
    TEST_ASSERT(struct_def.get_class() == XmlNodeClass::STRUCT);
    TEST_ASSERT(struct_def.get_caption() == L"N_STRUCT");
    TEST_ASSERT(struct_def.get_description() == L"D_STRUCT");

    auto& string_def = *def_childs++;
    TEST_ASSERT(string_def.get_class() == XmlNodeClass::TRIVIAL);
    TEST_ASSERT(string_def.get_caption() == L"N_STRING");
    TEST_ASSERT(string_def.get_description() == L"D_STRING");

    auto& blah_def = *def_childs++;
    TEST_EXCEPTION(blah_def.get_class());
    TEST_ASSERT(blah_def.get_caption() == XmlDefNode::get_default_caption());
    TEST_ASSERT(blah_def.get_description() == XmlDefNode::get_default_description());
}

void check_definition_test()
{
    std::string empty_list_def(
        "<?xml version=\"1.0\"?>"
        "<root class=\"STRUCT\">"
            "<list class=\"LIST\">"                             
            "</list>"
        "</root>"); 

    TEST_EXCEPTION(parse_definition(empty_list_def));

    std::string two_list_childs_def(
        "<?xml version=\"1.0\"?>"
        "<root class=\"STRUCT\">"
            "<list class=\"LIST\">"
                "<struct class=\"STRUCT\"/>"
                "<struct class=\"STRUCT\"/>"
            "</list>"
        "</root>"); 

    TEST_EXCEPTION(parse_definition(two_list_childs_def));

    std::string trivial_has_child_def(
        "<?xml version=\"1.0\"?>"
        "<root class=\"STRUCT\">"
            "<string class=\"STRING\">"
                "<string class=\"STRING\"/>"                
            "</string>"
        "</root>"); 

    TEST_EXCEPTION(parse_definition(trivial_has_child_def));
}

void struct_bindind_test()
{
    std::string xml_def(
        "<?xml version=\"1.0\"?>"
        "<struct class=\"STRUCT\">"
            "<string1 class=\"STRING\"/>"
            "<string2 class=\"STRING\"/>"
        "</struct>");   

    std::string xml_doc(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<string1/>"
            "<string2/>"
        "</struct>");

    auto def_doc = parse_definition(xml_def);
    auto doc = parse_regular(xml_doc, *def_doc);
    TEST_ASSERT(&doc->get_bound_def() == def_doc.get());

    auto struct_def = def_doc->get_root_def();
    auto& struct_node = cast_regular_node<XmlStructNode>(*doc->get_root());
    TEST_ASSERT(&struct_node.get_bound_def() == struct_def);

    auto struct_member_iterator = struct_node.begin_member();
    TEST_ASSERT(struct_member_iterator != struct_node.end_member());

    auto string_def_1 = begin_child(*struct_def);
    auto& string_node_1 = *struct_member_iterator++;
    TEST_ASSERT(&string_node_1.get_bound_def() == string_def_1);

    auto string_def_2 = next_child(*string_def_1);
    auto& string_node_2 = *struct_member_iterator++;
    TEST_ASSERT(&string_node_2.get_bound_def() == string_def_2);
    TEST_ASSERT(struct_member_iterator == struct_node.end_member());    
}

void list_bindind_test()
{
    std::string xml_def(
        "<?xml version=\"1.0\"?>"
        "<list class=\"LIST\">"
            "<string class=\"STRING\"/>"                    
        "</list>"); 

    std::string xml_doc(
        "<?xml version=\"1.0\"?>"
        "<list>"
            "<string/>"
            "<string/>"
            "<string/>"         
        "</list>");

    auto def = parse_definition(xml_def);
    auto doc = parse_regular(xml_doc, *def);
    
    TEST_ASSERT(&doc->get_bound_def() == def.get());

    auto list_def = cast_def_node<XmlListDef>(def->get_root_def());
    auto doc_root = doc->get_root();
    TEST_ASSERT(&doc_root->get_bound_def() == list_def);
    TEST_NO_EXCEPTION(cast_regular_node<XmlStringListNode>(*doc_root));
    
    auto& string_def = list_def->get_item_def(); 
    auto& list_node = cast_regular_node<XmlStringListNode>(*doc_root);
    auto list_item_iterator = list_node.begin_item();
    TEST_ASSERT(list_item_iterator != list_node.end_item())

    auto& string_node_1 = *list_item_iterator++;
    TEST_ASSERT(&string_node_1.get_bound_def() == &string_def);
    TEST_ASSERT(list_item_iterator != list_node.end_item());

    auto& string_node_2 = *list_item_iterator++;
    TEST_ASSERT(&string_node_2.get_bound_def() == &string_def);
    TEST_ASSERT(list_item_iterator != list_node.end_item());
    
    auto& string_node_3 = *list_item_iterator++;
    TEST_ASSERT(&string_node_3.get_bound_def() == &string_def);
    TEST_ASSERT(list_item_iterator == list_node.end_item());
}

void bindind_rules_test()
{
    std::string xml_def(
        "<?xml version=\"1.0\"?>"
        "<struct class=\"STRUCT\">"
            "<string1 class=\"STRING\"/>"   
            "<string2 class=\"STRING\"/>"
        "</struct>");   
    auto def = parse_definition(xml_def);

    std::string full_match_doc(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<string1/>"
            "<string2/>"
        "</struct>");
    
    auto doc = parse(full_match_doc);
    TEST_NO_EXCEPTION(bind_definition(*doc, *def));

    std::string additional_node_doc(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<string1/>"
            "<string2/>"
            "<string3/>"
        "</struct>");
    
    doc = parse(additional_node_doc);
    TEST_EXCEPTION(bind_definition(*doc, *def));

    std::string mismatch_name_doc(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<BLAH/>"
            "<string2/>"
        "</struct>");

    doc = parse(mismatch_name_doc);
    TEST_EXCEPTION(bind_definition(*doc, *def));

    std::string wrong_order_doc(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<string2/>"
            "<string1/>"        
        "</struct>");

    doc = parse(mismatch_name_doc);
    TEST_EXCEPTION(bind_definition(*doc, *def));    
}

} //namespace {

test_assist::test_suit get_xml_definition_suit()
{
    test_assist::test_suit ts(L"xml_definition_suit");
    ts
        .add({L"read_definition_test", &read_definition_test})  
        .add({L"check_definition_test", &check_definition_test})        
        .add({L"struct_bindind_test", &struct_bindind_test})
        .add({L"list_bindind_test", &list_bindind_test})
        .add({L"bindind_rules_test", &bindind_rules_test});
        
    return ts;
}