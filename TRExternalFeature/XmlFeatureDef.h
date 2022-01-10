#pragma once
#include "XmlReportTypeDef.h"
#include "XmlStreamTypeDef.h"
#include "XmlActionDef.h"
#include "XmlSourceTypeDef.h"
#include "XmlSelectionDef.h"
#include "XmlGroupingDef.h"
#include "XmlComplianceDef.h"
#include "XmlValidationDef.h"
#include "XmlDependenciesDef.h"
#include "stddecl.h"
namespace TR { namespace External {

struct XmlFeatureDef: XmlDoc
{
    static const XmlReportTypeDef* next_report_type_def(const XmlReportTypeDef* report_type_def);
    using ConstReportTypeDefIterator = xml_tools::node_iterator<const XmlReportTypeDef, next_report_type_def>;
    using ConstReportTypeDefRange = boost::iterator_range<ConstReportTypeDefIterator>;

    static const XmlStreamTypeDef* next_stream_type_def(const XmlStreamTypeDef* stream_type_def);
    using ConstStreamTypeDefIterator = xml_tools::node_iterator<const XmlStreamTypeDef, next_stream_type_def>;
    using ConstStreamTypeDefRange = boost::iterator_range<ConstStreamTypeDefIterator>;

    static const XmlActionDef* next_action_def(const XmlActionDef* action_def);
    using ConstActionDefIterator = xml_tools::node_iterator<const XmlActionDef, next_action_def>;
    using ConstActionDefRange = boost::iterator_range<ConstActionDefIterator>;

    static const XmlActionDelegateDef* next_action_delegate_def(const XmlActionDelegateDef* action_delegate_def);
    using ConstActionDelegateDefIterator = xml_tools::node_iterator<const XmlActionDelegateDef, next_action_delegate_def>;
    using ConstActionDelegateDefRange = boost::iterator_range<ConstActionDelegateDefIterator>;

    static const XmlActionShortcutDef* next_action_shortcut_def(const XmlActionShortcutDef* action_shortcut_def);
    using ConstActionShortcutDefIterator = xml_tools::node_iterator<const XmlActionShortcutDef, next_action_shortcut_def>;
    using ConstActionShortcutDefRange = boost::iterator_range<ConstActionShortcutDefIterator>;

    static const XmlSourceTypeDef* next_source_type_def(const XmlSourceTypeDef* source_type_def);
    using ConstSourceTypeDefIterator = xml_tools::node_iterator<const XmlSourceTypeDef, next_source_type_def>;
    using ConstSourceTypeDefRange = boost::iterator_range<ConstSourceTypeDefIterator>;

    static const XmlSelectionDef* next_selection_def(const XmlSelectionDef* selection_def);
    using ConstSelectionDefIterator = xml_tools::node_iterator<const XmlSelectionDef, next_selection_def>;
    using ConstSelectionDefRange = boost::iterator_range<ConstSelectionDefIterator>;

    static const XmlGroupingDef* next_grouping_def(const XmlGroupingDef* grouping_def);
    using ConstGroupingDefIterator = xml_tools::node_iterator<const XmlGroupingDef, next_grouping_def>;
    using ConstGroupingDefRange = boost::iterator_range<ConstGroupingDefIterator>;

    static const XmlComplianceDef* next_compliance_def(const XmlComplianceDef* compliance_def);
    using ConstComplianceDefIterator = xml_tools::node_iterator<const XmlComplianceDef, next_compliance_def>;
    using ConstComplianceDefRange = boost::iterator_range<ConstComplianceDefIterator>;

    static const XmlValidationDef* next_validation_def(const XmlValidationDef* validation_def);
    using ConstValidationDefIterator = xml_tools::node_iterator<const XmlValidationDef, next_validation_def>;
    using ConstValidationDefRange = boost::iterator_range<ConstValidationDefIterator>;

    UUID                            get_feature_uuid() const;
    std::wstring                    get_feature_name() const;   
    const XmlDependenciesDef*       get_dependencies() const;

    ConstReportTypeDefRange         get_report_types() const;
    ConstStreamTypeDefRange         get_stream_types() const;
    ConstActionDefRange             get_actions() const;
    ConstActionDelegateDefRange     get_action_delegates() const;
    ConstActionShortcutDefRange     get_action_shortcuts() const;
    ConstSourceTypeDefRange         get_source_types() const;
    ConstSelectionDefRange          get_selections() const;
    ConstGroupingDefRange           get_groupings() const;
    ConstComplianceDefRange         get_compliances() const;
    ConstValidationDefRange         get_validations() const;
};

std::unique_ptr<XmlFeatureDef>      open_feature_def(const std::wstring& path);

}}// namespace TR { namespace External {