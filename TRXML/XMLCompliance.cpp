#include "stdafx.h"
#include "XMLCompliance.h"
namespace TR { namespace XML {
using namespace xml_tools;

const XmlRuleDef* XmlComplianceDef::next_rule_def(const XmlRuleDef* rule_def)
{
    _ASSERT(rule_def);
    return static_cast<const XmlRuleDef*>(next_child(rule_def));
}

XmlComplianceDef::ConstRuleDefIterator XmlComplianceDef::begin_rule_def() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid compliance definition document");
    }

    return ConstRuleDefIterator(static_cast<const XmlRuleDef*>(begin_child(*root)));
}

XmlComplianceDef::ConstRuleDefIterator XmlComplianceDef::end_rule_def() const
{
    return ConstRuleDefIterator();
}

XmlComplianceDef::ConstRuleDefRange XmlComplianceDef::get_rule_defs() const
{
    return ConstRuleDefRange(begin_rule_def(), end_rule_def());
}

XmlRuleDef::Type XmlRuleDef::get_type() const
{
    switch (get_class())
    {
    case XmlNodeClass::STRUCT:
        return Type::CHECKING;

    case XmlNodeClass::LIST:
        return Type::LISTING;

    default:
        _ASSERT(false);
    }

    throw std::logic_error("Invalid compliance definition document");
}

const XmlRuleNode* XmlComplianceDoc::next_rule(const XmlRuleNode* rule_node)
{
    _ASSERT(rule_node);
    return static_cast<const XmlRuleNode*>(next_child(rule_node));
}

XmlComplianceDoc::ConstRuleIterator XmlComplianceDoc::begin_rule() const
{
    auto root = get_root();
    if (!root)
    {
        throw std::logic_error("Invalid compliance document");
    }

    return ConstRuleIterator(static_cast<const XmlRuleNode*>(begin_child(*root)));
}

XmlComplianceDoc::ConstRuleIterator XmlComplianceDoc::end_rule() const
{
    return ConstRuleIterator();
}

XmlComplianceDoc::ConstRuleRange XmlComplianceDoc::get_rules() const
{
    return ConstRuleRange(begin_rule(), end_rule());
}

bool XmlRuleCheck::is_checked() const
{
    auto member_node = begin_member();
    if (member_node == end_member())
    {
        throw std::logic_error("Invalid compliance document");
    }

    auto& checked_node = cast_regular_node<const XmlBooleanNode>(*member_node);
    return checked_node.get_value();
}

bool XmlRuleCheck::is_excepted() const
{
    auto first_member_node = begin_member();
    if (first_member_node == end_member())
    {
        throw std::logic_error("Invalid compliance document");
    }

    auto second_member_node = ++first_member_node;
    if (second_member_node == end_member())
    {
        throw std::logic_error("Invalid compliance document");
    }

    auto& value_node = cast_regular_node<const XmlStringNode>(*second_member_node);
    if (auto condition = find_attribute(value_node, "condition"))
    {
        _ASSERT(attr_value_equal(*condition, "exception"));
        return true;
    }

    return false;
}

std::wstring XmlRuleCheck::get_id() const
{
    auto first_member_node = begin_member();
    if (first_member_node == end_member())
    {
        throw std::logic_error("Invalid compliance document");
    }

    auto second_member_node = ++first_member_node;
    if (second_member_node == end_member())
    {
        throw std::logic_error("Invalid compliance document");
    }
    
    auto& id_node = cast_regular_node<const XmlStringNode>(*second_member_node);
    return id_node.get_value();
}

const XmlRuleCheck* XmlRuleNode::next_rule_check(const XmlRuleCheck* rule_check)
{
    _ASSERT(rule_check);
    return static_cast<const XmlRuleCheck*>(next_child(rule_check));
}

const XmlRuleDef& XmlRuleNode::get_bound_def() const
{
    return static_cast<const XmlRuleDef&>(XmlRegularNode::get_bound_def());
}

XmlRuleNode::CheckResult XmlRuleNode::get_check_result() const
{
    auto check_result = (get_bound_def().get_type() == XmlRuleDef::Type::CHECKING) ? CheckResult::UNDEFINED : CheckResult::SUCCEEDED;
    
    for (auto& rule_check : get_rule_checks())
    {
        if (rule_check.is_excepted())
        {
            continue;
        }

        if (rule_check.is_checked())
        {
            check_result = CheckResult::SUCCEEDED;
            continue;
        }

        check_result = CheckResult::FAILED;
        break;
    }
    return check_result;    
}

XmlRuleNode::ConstRuleCheckIterator XmlRuleNode::begin_rule_check() const
{
    auto& rule_def = get_bound_def();
    switch (rule_def.get_type())
    {
    case XmlRuleDef::Type::CHECKING:
    {
        auto& struct_node = cast_regular_node<const XmlStructNode>(*this);
        return ConstRuleCheckIterator(static_cast<const XmlRuleCheck*>(&struct_node));
    }

    case XmlRuleDef::Type::LISTING:
    {
        auto& checking_list_node = cast_regular_node<const XmlStructListNode>(*this);
        if (checking_list_node.empty())
        {
            return ConstRuleCheckIterator();
        }
        return ConstRuleCheckIterator(static_cast<const XmlRuleCheck*>(&*checking_list_node.begin_item()));
    }

    default:
        _ASSERT(false);
    }

    throw std::logic_error("Invalid compliance document");
}

XmlRuleNode::ConstRuleCheckIterator XmlRuleNode::end_rule_check() const
{
    if (get_bound_def().get_type() == XmlRuleDef::Type::CHECKING)
    {
        return ++begin_rule_check();
    }
    return ConstRuleCheckIterator();
}

XmlRuleNode::ConstRuleCheckRange XmlRuleNode::get_rule_checks() const
{
    return ConstRuleCheckRange(begin_rule_check(), end_rule_check());
}

XmlRuleSetup* XmlComplianceSetup::next_rule_setup(XmlRuleSetup* xml_rule_setup)
{
    _ASSERT(xml_rule_setup);
    return static_cast<XmlRuleSetup*>(next_child(xml_rule_setup));
}

const XmlRuleSetup* XmlComplianceSetup::next_rule_setup(const XmlRuleSetup* xml_rule_setup)
{
    return next_rule_setup(const_cast<XmlRuleSetup*>(xml_rule_setup));
}

XmlComplianceSetup::RuleSetupIterator XmlComplianceSetup::begin_rule_setup()
{
    auto root_query = get_root();
    if (!root_query)
    {
        throw std::logic_error("Invalid compliance setup document");
    }
    return RuleSetupIterator(static_cast<XmlRuleSetup*>(begin_child(*root_query)));
}

XmlComplianceSetup::RuleSetupIterator XmlComplianceSetup::end_rule_setup()
{
    return RuleSetupIterator();
}

XmlComplianceSetup::RuleSetupRange XmlComplianceSetup::get_rule_setups()
{
    return RuleSetupRange(begin_rule_setup(), end_rule_setup());
}

XmlComplianceSetup::ConstRuleSetupIterator XmlComplianceSetup::begin_rule_setup() const
{
    auto root_query = get_root();
    if (!root_query)
    {
        throw std::logic_error("Invalid compliance setup document");
    }

    return ConstRuleSetupIterator(static_cast<const XmlRuleSetup*>(begin_child(*root_query)));
}

XmlComplianceSetup::ConstRuleSetupIterator XmlComplianceSetup::end_rule_setup() const
{
    return ConstRuleSetupIterator();
}

XmlComplianceSetup::ConstRuleSetupRange XmlComplianceSetup::get_rule_setups() const
{
    return ConstRuleSetupRange(begin_rule_setup(), end_rule_setup());
};

const XmlRuleDef& XmlRuleSetup::get_bound_def() const
{
    return static_cast<const XmlRuleDef&>(XmlQueryNode::get_bound_def());
}

XmlRuleSetup::CheckingPolicy XmlRuleSetup::get_checking_policy() const
{
    if (!is_action_specified())
    {
        return CHECKING_DEFAULT;
    }

    switch (get_action())
    {
    case Action::ACCEPT:
        return ENABLE_CHECKING;

    case Action::REJECT:
        return DISABLE_CHECKING;

    default:
        _ASSERT(false);
    }

    throw std::logic_error("XmlRuleFilter : Invalid `Action` attribute value");
}

void XmlRuleSetup::set_checking_policy(CheckingPolicy checking_policy)
{
    switch (checking_policy)
    {
    case CHECKING_DEFAULT:
        reset_action();
        break;
    
    case ENABLE_CHECKING:
        set_action(Action::ACCEPT);
        break;

    case DISABLE_CHECKING:
        set_action(Action::REJECT);
        break;

    default:
        _ASSERT(false);
    }
}

XmlCompositeQueryNode& get_checking_setup_node(XmlRuleSetup& xml_rule_setup)
{
    auto& rule_def = xml_rule_setup.get_bound_def();
    switch (rule_def.get_type())
    {
    case XmlRuleDef::Type::CHECKING:
        return xml_rule_setup;

    case XmlRuleDef::Type::LISTING:
    {
        auto checking_setup_node = xml_rule_setup.begin_child_node();
        if (checking_setup_node == xml_rule_setup.end_child_node())
        {
            throw std::logic_error("Invalid compliance setup document");
        }
        return static_cast<XmlCompositeQueryNode&>(*checking_setup_node);
    }

    default:
        _ASSERT(false);
    }
    throw std::logic_error("Invalid compliance setup document");
}

XmlConditionalQueryNode& get_value_setup_node(XmlRuleSetup& rule_setup)
{
    auto& checking_setup_node = get_checking_setup_node(rule_setup);

    auto checked_node = checking_setup_node.begin_child_node();
    if (checked_node == checking_setup_node.end_child_node())
    {
        throw std::logic_error("Invalid compliance setup document");
    }

    auto value_node = ++checked_node;
    if (value_node == checking_setup_node.end_child_node())
    {
        throw std::logic_error("Invalid compliance setup document");
    }

    return static_cast<XmlConditionalQueryNode&>(*value_node);
}

void XmlRuleSetup::set_exceptions(const std::vector<std::wstring>& exceptions)
{
    auto& value_setup_node = get_value_setup_node(*this);
    remove_children(value_setup_node);
    if (!exceptions.empty())
    {
        value_setup_node.add_condition(create_condition("exception", XmlConditionNode::Action::ACCEPT, XmlConditionNode::Predicate::EQUAL, exceptions));
    }
}

std::vector<std::wstring> XmlRuleSetup::get_exceptions() const
{
    const auto& value_setup = get_value_setup_node(const_cast<XmlRuleSetup&>(*this));
    
    auto condition_node = value_setup.begin_condition();
    if (condition_node == value_setup.end_condition())
    {
        return{};
    }

    return condition_node->get_patterns();  
}

// create functions
std::unique_ptr<XmlComplianceDef> parse_compliance_def(const std::string& xml_compliance_def)
{
    auto def_doc = stl_tools::static_pointer_cast<XmlComplianceDef>(create_doc());  
    auto xml_doc = parse(xml_compliance_def);
    auto root = get_root(*xml_doc);
    auto& def_root = set_root(*def_doc, create_element(get_node_name(*root).c_str()));
    set_attr_value(def_root, "class", "STRUCT");

    for (auto& child_node : get_child_nodes(*root))
    {
        auto& rule_def_node = add_element(def_root, get_node_name(child_node).c_str());
        set_attr_value(rule_def_node, "optional", "yes");

        if (auto caption = find_attribute(child_node, "caption"))
        {
            set_attr_value(rule_def_node, "caption", get_attr_value(*caption).c_str());
        }

        if (auto description = find_attribute(child_node, "description"))
        {
            set_attr_value(rule_def_node, "description", get_attr_value(*description).c_str());
        }
            
        XmlRuleDef::Type rule_type = XmlRuleDef::Type::CHECKING;
        if (auto type = find_attribute(child_node, "type"))
        {
            if (attr_value_equal(*type, "LISTING"))
            {
                rule_type = XmlRuleDef::Type::LISTING;
            }
            else
            {
                _ASSERT(attr_value_equal(*type, "CHECKING"));
            }
        }

        if (rule_type == XmlRuleDef::Type::CHECKING)
        {
            set_attr_value(rule_def_node, "class", "STRUCT");
            
            auto& result_def_node = add_element(rule_def_node, "result");
            set_attr_value(result_def_node, "class", "BOOLEAN");
            if (auto result = find_attribute(child_node, "result"))
            {
                set_attr_value(result_def_node, "caption", get_attr_value(*result).c_str());
            }
            else
            {
                set_attr_value(result_def_node, "caption", "Result");
            }

            auto& value_def_node = add_element(rule_def_node, "value");
            set_attr_value(value_def_node, "class", "STRING");
            if (auto value = find_attribute(child_node, "value"))
            {
                set_attr_value(value_def_node, "caption", get_attr_value(*value).c_str());
            }
            else
            {
                set_attr_value(value_def_node, "caption", "Value");
            }

        }
        else if (rule_type == XmlRuleDef::Type::LISTING)
        {
            set_attr_value(rule_def_node, "class", "LIST");         

            auto& checking_def_node = add_element(rule_def_node, "checking");
            set_attr_value(checking_def_node, "class", "STRUCT");


            auto& result_def_node = add_element(checking_def_node, "result");
            set_attr_value(result_def_node, "class", "BOOLEAN");
            if (auto result = find_attribute(child_node, "result"))
            {
                set_attr_value(result_def_node, "caption", get_attr_value(*result).c_str());
            }
            else
            {
                set_attr_value(result_def_node, "caption", "Result");
            }

            auto& value_def_node = add_element(checking_def_node, "value");
            set_attr_value(value_def_node, "class", "STRING");
            if (auto value = find_attribute(child_node, "value"))
            {
                set_attr_value(value_def_node, "caption", get_attr_value(*value).c_str());
            }
            else
            {
                set_attr_value(value_def_node, "caption", "Value");
            }
        }
    }

    return def_doc;
}

std::unique_ptr<XmlComplianceDoc> parse_compliance_doc(const std::string& xml_compliance_doc, const XmlComplianceDef& def_doc)
{
    return stl_tools::static_pointer_cast<XmlComplianceDoc>(parse_regular(xml_compliance_doc, def_doc));
}

std::unique_ptr<XmlComplianceSetup> parse_compliance_setup(const std::string& xml_compliance_setup, const XmlComplianceDef& def_doc)
{
    return stl_tools::static_pointer_cast<XmlComplianceSetup>(parse_query(xml_compliance_setup, def_doc));
}

std::unique_ptr<XmlComplianceSetup> create_compliance_setup(const XmlComplianceDef& def_doc)
{
    auto compliance_setup = stl_tools::static_pointer_cast<XmlComplianceSetup>(create_query(def_doc));
    for (auto& rule_setup : compliance_setup->get_rule_setups())
    {
        auto& value_setup = get_value_setup_node(rule_setup);
        value_setup.set_default(XmlQueryNode::Action::ACCEPT);
    }
    return compliance_setup;
}

}} //namespace TR { namespace XML {