#pragma once
#include "XMLDefinition.h"
#include "XMLRegular.h"
#include "XMLQuery.h"
#include "TRXMLStd.h"
namespace TR { namespace XML {

struct XmlRuleDef;
struct XmlComplianceDef: XmlDefDoc
{
    static const XmlRuleDef* next_rule_def(const XmlRuleDef*);
    using ConstRuleDefIterator = xml_tools::node_iterator<const XmlRuleDef, next_rule_def>;
    using ConstRuleDefRange = boost::iterator_range<ConstRuleDefIterator>;

    ConstRuleDefIterator            begin_rule_def() const;
    ConstRuleDefIterator            end_rule_def() const;
    ConstRuleDefRange               get_rule_defs() const;
};

struct XmlRuleDef: XmlDefNode
{
    enum class Type { CHECKING, LISTING };
    Type get_type() const;
};

struct XmlRuleNode;
struct XmlComplianceDoc: XmlRegularDoc
{
    static const XmlRuleNode* next_rule(const XmlRuleNode*);
    using ConstRuleIterator = xml_tools::node_iterator<const XmlRuleNode, next_rule>;
    using ConstRuleRange = boost::iterator_range<ConstRuleIterator>;

    ConstRuleIterator               begin_rule() const;
    ConstRuleIterator               end_rule() const;
    ConstRuleRange                  get_rules() const;
};

struct XmlRuleCheck: XmlStructNode
{
    bool                            is_checked() const;
    bool                            is_excepted() const;
    std::wstring                    get_id() const;
};

struct XmlRuleNode: XmlRegularNode
{
    enum class CheckResult { CHECKED = 0, FAILED, EXCEPTED };

    static const XmlRuleCheck* next_rule_check(const XmlRuleCheck*);
    using ConstRuleCheckIterator = xml_tools::node_iterator<const XmlRuleCheck, next_rule_check>;
    using ConstRuleCheckRange = boost::iterator_range<ConstRuleCheckIterator>;

    const XmlRuleDef&               get_bound_def() const;
    CheckResult                     get_check_result() const;

    ConstRuleCheckIterator          begin_rule_check() const;
    ConstRuleCheckIterator          end_rule_check() const;
    ConstRuleCheckRange             get_rule_checks() const;
};

struct XmlRuleSetup;
struct XmlComplianceSetup: XmlQueryDoc
{
    static XmlRuleSetup* next_rule_setup(XmlRuleSetup*);
    using RuleSetupIterator = xml_tools::node_iterator<XmlRuleSetup, next_rule_setup>;
    using RuleSetupRange = boost::iterator_range<RuleSetupIterator>;

    static const XmlRuleSetup* next_rule_setup(const XmlRuleSetup*);
    using ConstRuleSetupIterator = xml_tools::node_iterator<const XmlRuleSetup, next_rule_setup>;
    using ConstRuleSetupRange = boost::iterator_range<ConstRuleSetupIterator>;

    RuleSetupIterator               begin_rule_setup();
    RuleSetupIterator               end_rule_setup();
    RuleSetupRange                  get_rule_setups();

    ConstRuleSetupIterator          begin_rule_setup() const;
    ConstRuleSetupIterator          end_rule_setup() const;
    ConstRuleSetupRange             get_rule_setups() const;
};

struct XmlRuleSetup: XmlCompositeQueryNode
{
    enum CheckingPolicy { CHECKING_DEFAULT = 0, ENABLE_CHECKING, DISABLE_CHECKING };

    const XmlRuleDef&               get_bound_def() const;
    
    CheckingPolicy                  get_checking_policy() const;
    void                            set_checking_policy(CheckingPolicy checking_policy);    

    std::vector<std::wstring>       get_exceptions() const;
    void                            set_exceptions(const std::vector<std::wstring>& exceptions);
};

// create functions
std::unique_ptr<XmlComplianceDef>       parse_compliance_def(const std::string& xml_compliance_def);
std::unique_ptr<XmlComplianceDoc>       parse_compliance_doc(const std::string& xml_compliance_doc, const XmlComplianceDef& comliance_def);
std::unique_ptr<XmlComplianceSetup>     parse_compliance_setup(const std::string& xml_compliance_setup, const XmlComplianceDef& comliance_def);
std::unique_ptr<XmlComplianceSetup>     create_compliance_setup(const XmlComplianceDef& def_doc);

}} //namespace TR { namespace XML {