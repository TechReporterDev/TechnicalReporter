#include "stdafx.h"
#include "XMLValidation.h"
namespace TR { namespace XML {
using namespace xml_tools;

bool XmlValidatedNode::is_valid() const
{
    if (auto condition = find_attribute(*this, "condition"))
    {
        if (attr_value_equal(*condition, "valid"))
        {
            return true;
        }

        _ASSERT(attr_value_equal(*condition, "invalid"));
        return false;
    }
    return true;
}

std::wstring XmlValidatedNode::get_comment() const
{
    if (auto comment = find_attribute(*this, "comment"))
    {
        return get_attr_wvalue(*comment);
    }
    return L"";
}

void XmlValidationNode::set_decision(Decision decision)
{
    switch (decision)
    {
    case VALID:
        set_name("valid");
        break;
    case INVALID:
        set_name("invalid");        
        break;

    default:
        _ASSERT(false);
    }
}

XmlValidationNode::Decision XmlValidationNode::get_decision() const
{
    if (*get_name() == "valid")
    {
        return VALID;
    }

    _ASSERT(*get_name() == "invalid");
    return INVALID;
}

std::unique_ptr<XmlValidationNode> create_validation(XmlValidationNode::Decision desition, XmlConditionNode::Predicate predicate, const std::vector<std::wstring>& patterns, const std::string& comment)
{
    auto validation_node = stl_tools::static_pointer_cast<XmlValidationNode>(create_condition(XmlConditionNode::Action::ACCEPT, predicate, patterns));
    validation_node->set_decision(desition);
    validation_node->set_comment(comment);
    return validation_node;
}

struct invalidate_by_default: boost::static_visitor<void>
{
    void operator ()(XmlCompositeQueryNode& composite_query) const
    {
        for (auto& child_query : composite_query.get_child_nodes())
        {
            visit_query_node(*this, child_query);
        }
    }

    void operator ()(XmlConditionalQueryNode& conditional_query) const
    {
        conditional_query.set_default(XmlQueryNode::Action::ACCEPT, std::string("invalid"), std::string("Value does not match any condition"));
    }   
};

std::unique_ptr<XmlQueryDoc> create_validator(const XmlDefDoc& def_doc)
{
    auto validator_doc = create_query(def_doc);
    visit_query_node(invalidate_by_default(), *validator_doc->get_root());
    return validator_doc;
}

std::unique_ptr<XmlQueryDoc> parse_validator(const std::string& xml_validator, const XmlDefDoc& def_doc)
{
    return stl_tools::static_pointer_cast<XmlQueryDoc>(parse_query(xml_validator, def_doc));
}

}} //namespace TR { namespace XML {