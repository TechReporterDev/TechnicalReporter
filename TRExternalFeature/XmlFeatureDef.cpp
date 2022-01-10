#include "stdafx.h"
#include "XmlFeatureDef.h"
#include "W32Cmd.h"
#include <boost\filesystem.hpp>

namespace TR { namespace External {
using namespace xml_tools;
namespace bfs = boost::filesystem;

boost::optional<std::wstring> get_external_def(XmlNode* feature_node)
{
    if (auto external_def = find_child(*feature_node, "external_definition"))
    {
        return get_node_wtext(*external_def);
    }
    return boost::none; 
}

const XmlReportTypeDef* XmlFeatureDef::next_report_type_def(const XmlReportTypeDef* report_type_def)
{
    _ASSERT(report_type_def && node_name_equal(*report_type_def, "report_type"));
    return static_cast<const XmlReportTypeDef*>(find_sibling(*report_type_def, "report_type"));
}

const XmlStreamTypeDef* XmlFeatureDef::next_stream_type_def(const XmlStreamTypeDef* stream_type_def)
{
    _ASSERT(stream_type_def && (node_name_equal(*stream_type_def, "stream_type")));

    for (auto sibling_node = next_child(stream_type_def); sibling_node != nullptr; sibling_node = next_child(sibling_node))
    {
        if (node_name_equal(*sibling_node, "stream_type"))
        {
            return static_cast<const XmlStreamTypeDef*>(sibling_node);
        }
    }
    return nullptr;
}

const XmlActionDef* XmlFeatureDef::next_action_def(const XmlActionDef* action_def)
{
    _ASSERT(action_def && node_name_equal(*action_def, "action"));
    return static_cast<const XmlActionDef*>(find_sibling(*action_def, "action"));
}

const XmlActionDelegateDef* XmlFeatureDef::next_action_delegate_def(const XmlActionDelegateDef* action_delegate_def)
{
    _ASSERT(action_delegate_def && node_name_equal(*action_delegate_def, "action_delegate"));
    return static_cast<const XmlActionDelegateDef*>(find_sibling(*action_delegate_def, "action_delegate"));
}

const XmlActionShortcutDef* XmlFeatureDef::next_action_shortcut_def(const XmlActionShortcutDef* action_shortcut_def)
{
    _ASSERT(action_shortcut_def && node_name_equal(*action_shortcut_def, "action_shortcut"));
    return static_cast<const XmlActionShortcutDef*>(find_sibling(*action_shortcut_def, "action_shortcut"));
}

const XmlSourceTypeDef* XmlFeatureDef::next_source_type_def(const XmlSourceTypeDef* source_type_def)
{
    _ASSERT(source_type_def && node_name_equal(*source_type_def, "source_type"));
    return static_cast<const XmlSourceTypeDef*>(find_sibling(*source_type_def, "source_type"));
}

const XmlSelectionDef* XmlFeatureDef::next_selection_def(const XmlSelectionDef* selection_node)
{
    _ASSERT(selection_node && node_name_equal(*selection_node, "selection"));
    return static_cast<const XmlSelectionDef*>(find_sibling(*selection_node, "selection"));
}

const XmlGroupingDef* XmlFeatureDef::next_grouping_def(const XmlGroupingDef* grouping_def)
{
    _ASSERT(grouping_def && node_name_equal(*grouping_def, "grouping"));
    return static_cast<const XmlGroupingDef*>(find_sibling(*grouping_def, "grouping"));
}

const XmlComplianceDef* XmlFeatureDef::next_compliance_def(const XmlComplianceDef* compliance_def)
{
    _ASSERT(compliance_def && node_name_equal(*compliance_def, "compliance"));
    return static_cast<const XmlComplianceDef*>(find_sibling(*compliance_def, "compliance"));
}

const XmlValidationDef* XmlFeatureDef::next_validation_def(const XmlValidationDef* validation_def)
{
    _ASSERT(validation_def && node_name_equal(*validation_def, "validation"));
    return static_cast<const XmlValidationDef*>(find_sibling(*validation_def, "validation"));
}

UUID XmlFeatureDef::get_feature_uuid() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto uuid_node = find_child(*root, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid feature definition document");
    }
    return stl_tools::gen_uuid(get_node_wtext(*uuid_node));
}

std::wstring XmlFeatureDef::get_feature_name() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto name_node = find_child(*root, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid feature definition document");
    }
    return get_node_wtext(*name_node);
}

const XmlDependenciesDef* XmlFeatureDef::get_dependencies() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    return static_cast<const XmlDependenciesDef*>(find_child(*root, "dependencies"));
}

XmlFeatureDef::ConstReportTypeDefRange XmlFeatureDef::get_report_types() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstReportTypeDefIterator(static_cast<const XmlReportTypeDef*>(find_child(*root, "report_type")));
    return ConstReportTypeDefRange(begin, ConstReportTypeDefIterator());
}

XmlFeatureDef::ConstStreamTypeDefRange XmlFeatureDef::get_stream_types() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    ConstStreamTypeDefIterator begin;
    for (auto child_node = begin_child(*root); child_node != nullptr; child_node = next_child(child_node))
    {
        if (node_name_equal(*child_node, "stream_type"))
        {
            begin = ConstStreamTypeDefIterator(static_cast<const XmlStreamTypeDef*>(child_node));
        }
    }
    return ConstStreamTypeDefRange(begin, ConstStreamTypeDefIterator());
}

XmlFeatureDef::ConstActionDefRange XmlFeatureDef::get_actions() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstActionDefIterator(static_cast<const XmlActionDef*>(find_child(*root, "action")));
    return ConstActionDefRange(begin, ConstActionDefIterator());
}

XmlFeatureDef::ConstActionDelegateDefRange XmlFeatureDef::get_action_delegates() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstActionDelegateDefIterator(static_cast<const XmlActionDelegateDef*>(find_child(*root, "action_delegate")));
    return ConstActionDelegateDefRange(begin, ConstActionDelegateDefIterator());
}

XmlFeatureDef::ConstActionShortcutDefRange XmlFeatureDef::get_action_shortcuts() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstActionShortcutDefIterator(static_cast<const XmlActionShortcutDef*>(find_child(*root, "action_shortcut")));
    return ConstActionShortcutDefRange(begin, ConstActionShortcutDefIterator());
}

XmlFeatureDef::ConstSourceTypeDefRange XmlFeatureDef::get_source_types() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstSourceTypeDefIterator(static_cast<const XmlSourceTypeDef*>(find_child(*root, "source_type")));
    return ConstSourceTypeDefRange(begin, ConstSourceTypeDefIterator());
}

XmlFeatureDef::ConstSelectionDefRange XmlFeatureDef::get_selections() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstSelectionDefIterator(static_cast<const XmlSelectionDef*>(find_child(*root, "selection")));
    return ConstSelectionDefRange(begin, ConstSelectionDefIterator());
}

XmlFeatureDef::ConstGroupingDefRange XmlFeatureDef::get_groupings() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstGroupingDefIterator(static_cast<const XmlGroupingDef*>(find_child(*root, "grouping")));
    return ConstGroupingDefRange(begin, ConstGroupingDefIterator());
}

XmlFeatureDef::ConstComplianceDefRange XmlFeatureDef::get_compliances() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstComplianceDefIterator(static_cast<const XmlComplianceDef*>(find_child(*root, "compliance")));
    return ConstComplianceDefRange(begin, ConstComplianceDefIterator());
}

XmlFeatureDef::ConstValidationDefRange XmlFeatureDef::get_validations() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    auto begin = ConstValidationDefIterator(static_cast<const XmlValidationDef*>(find_child(*root, "validation")));
    return ConstValidationDefRange(begin, ConstValidationDefIterator());
}

void expand_external(XmlFeatureDef& feature_def, const std::wstring& search_dir)
{
    auto feature_root = get_root(feature_def);
    if (!feature_root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    for (auto& feature_node : get_child_nodes(*feature_root))
    {
        if (auto external_name = get_external_def(&feature_node))
        {
            bfs::path external_path(*external_name);
            if (!bfs::exists(external_path) && external_path.is_relative())
            {
                external_path = bfs::path(search_dir) / external_path;
            }

            auto external_def = parse(read_text(external_path.wstring()));
            auto external_root = get_root(*external_def);

            if (!external_root)
            {
                throw std::logic_error("Invalid feature definition document");
            }

            if (!node_name_equal(feature_node, *external_root))
            {
                throw std::logic_error("Invalid feature definition document");
            }

            for (auto& external_child : get_child_nodes(*external_root))
            {
                clone_into(external_child, feature_node);
            }
        }
    }
}

boost::optional<std::pair<std::string, std::string>> get_tag(const XmlNode& node)
{
    auto tag_node = find_child(node, "tag");
    if (!tag_node)
    {
        return boost::none;
    }

    auto uuid_node = find_child(node, "uuid");
    if (!uuid_node)
    {
        return boost::none;
    }

    return std::make_pair(get_node_text(*tag_node), get_node_text(*uuid_node));
}

void collect_tags(const XmlNode& parent_node, std::map<std::string, std::string>& tags)
{
    if (auto tag = get_tag(parent_node))
    {
        if (!tags.insert(*tag).second)
        {
            throw std::logic_error("Duplicated tag found");
        }
    }

    for (auto& child_node : get_child_nodes(parent_node, XmlNodeType::ELEMENT))
    {
        collect_tags(child_node, tags);
    }
}

std::map<std::string, std::string> collect_tags(const XmlFeatureDef& feature_def)
{
    auto feature_root = get_root(feature_def);
    if (!feature_root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    std::map<std::string, std::string> tags;
    collect_tags(*feature_root, tags);  
    return tags;
}

void resolve_tags(XmlNode& feature_node, const std::map<std::string, std::string>& tags)
{   
    auto child_node = begin_child(feature_node);
    for (auto child_node = begin_child(feature_node); child_node != nullptr;)
    {
        auto next_node = next_child(child_node);
        if (node_name_equal(*child_node, "uuid-of"))
        {
            auto tag = get_node_text(*child_node);
            replace_node(*child_node, create_text_node(tags.at(tag).c_str()));
        }
        else
        {
            resolve_tags(*child_node, tags);
        }
        child_node = next_node;
    }   
}

void resolve_tags(XmlFeatureDef& feature_def)
{
    auto feature_root = get_root(feature_def);
    if (!feature_root)
    {
        throw std::logic_error("Invalid feature definition document");
    }

    resolve_tags(*feature_root, collect_tags(feature_def));
}

std::unique_ptr<XmlFeatureDef> open_feature_def(const std::wstring& path)
{
    auto feature_def = stl_tools::static_pointer_cast<XmlFeatureDef>(xml_tools::parse(read_text(path)));
    expand_external(*feature_def, boost::filesystem::path(path).parent_path().wstring());
    resolve_tags(*feature_def);
    return feature_def;
}

}}// namespace TR { namespace External {