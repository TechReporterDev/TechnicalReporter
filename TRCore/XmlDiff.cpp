#include "stdafx.h"
#include "XmlDiff.h"
#include <boost/algorithm/string.hpp>
#include <libxmldiff/libxmldiff.h>
namespace TR { namespace Core {
using namespace xml_tools;

XmlDiffPoint::Type XmlDiffPoint::get_type() const
{
    auto size = m_path.size();
    _ASSERT(size);
    auto last = size - 1;
    
    if (m_path[last].is_attr())
    {
        return ATTRIBUTE;
    }
    else if (m_path[last].is_element())
    {
        return ELEMENT;
    }

    else
    {
        _ASSERT(m_path[last].is_value());
        
        _ASSERT(size > 1);
        auto last_but_one = last - 1;       
        if (m_path[last_but_one].is_attr())
        {
            return XmlDiffPoint::ATTR_VALUE;
        }
        else
        {
            _ASSERT(m_path[last_but_one].is_element());
            return XmlDiffPoint::TEXT;
        }
    }

    _ASSERT(false);
    return ELEMENT; //to supress warning
}

DN_STATUS get_status(const XmlNode& diff_node)
{
    if (auto status_attr = find_attribute(diff_node, "diff", "status"))
    {
        if (attr_value_equal(*status_attr, "DN_BELOW"))
        {
            return DN_BELOW;
        }

        else if(attr_value_equal(*status_attr, "DN_ADDED"))
        {
            return DN_ADDED;                
        }

        else if (attr_value_equal(*status_attr, "DN_REMOVED"))
        {
            return DN_REMOVED;
        }

        else if (attr_value_equal(*status_attr, "DN_MODIFIED"))
        {
            return DN_MODIFIED;
        }       
    }
    return DN_NONE;
}

boost::optional<std::string> get_node_id(const XmlNode& diff_node, const std::string& id)
{
    if (auto id = find_attribute(diff_node, "id"))
    {
        return get_attr_value(*id);
    }
    return boost::none;
}

boost::optional<std::pair<std::string, std::string>> split_value(const std::string& value)
{   
    if (auto range = boost::find_first(value, "%%%"))
    {
        return std::make_pair(std::string(value.begin(), range.begin()), std::string(range.end(), value.end()));
    }
    return boost::none;
}

std::vector<XmlDiffPoint> compress_diff(const std::string& text)
{
    std::vector<XmlDiffPoint> diff_points;
    if (auto values = split_value(text))
    {
        std::string new_value;
        std::string old_value;
        std::tie(new_value, old_value) = std::move(*values);

        XmlDiffPoint removed(XmlDiffPoint::STATE_REMOVED);
        removed.m_path.push_back(XmlNodeData(""));
        removed.m_path.back().m_value = std::move(old_value);
        diff_points.push_back(std::move(removed));

        XmlDiffPoint inserted(XmlDiffPoint::STATE_INSERTED);
        inserted.m_path.push_back(XmlNodeData(""));
        inserted.m_path.back().m_value = std::move(new_value);
        diff_points.push_back(std::move(inserted));
    }
    return diff_points;
}

std::vector<XmlDiffPoint> compress_diff(const xmlAttr& attr)
{
    std::vector<XmlDiffPoint> diff_points = compress_diff(get_attr_value(attr));
    if (diff_points.empty())
    {
        return {};
    }

    XmlNodeData attr_node_data("@" + get_attr_name(attr));
    for (auto& diff_point : diff_points)
    {
        diff_point.m_path.push_front(attr_node_data);
    }
    return diff_points;
}

std::vector<XmlDiffPoint> compress_diff(const XmlNode& diff_node, const std::string& id)
{
    auto status = get_status(diff_node);
    if (status == DN_NONE)
    {
        return{};
    }

    std::vector<XmlDiffPoint> diff_points;
    XmlNodeData node_data(get_node_name(diff_node));
    node_data.m_id = get_node_id(diff_node, id);    

    switch (status)
    {
    case DN_ADDED:
        {
            XmlDiffPoint inserted(XmlDiffPoint::STATE_INSERTED);
            inserted.m_path.push_back(node_data);
            diff_points.push_back(std::move(inserted));
            break;
        }

    case DN_REMOVED:
        {
            XmlDiffPoint removed(XmlDiffPoint::STATE_REMOVED);
            removed.m_path.push_back(node_data);
            diff_points.push_back(std::move(removed));
            break;
        }

    case DN_MODIFIED:
        {
            for (auto& diff_point : compress_diff(get_node_text(diff_node)))
            {
                diff_point.m_path.push_front(node_data);
                diff_points.push_back(std::move(diff_point));
            }

            int position = 0;
            for (auto attr = begin_attribute(diff_node); attr != nullptr; attr = next_attribute(*attr))
            {
                for (auto& diff_point : compress_diff(*attr))
                {
                    _ASSERT(diff_point.m_path.front().m_position == 0);
                    diff_point.m_path.front().m_position = position;
                    diff_point.m_path.push_front(node_data);
                    diff_points.push_back(std::move(diff_point));
                }
                ++position;
            }       

            //break; //no break, bacause DN_MODIFIED can override DN_BELOW
        }

    case DN_BELOW:
        {
            int position = 0;
            for (auto& child : get_child_nodes(diff_node))
            {
                for (auto& diff_point : compress_diff(child, id))
                {
                    _ASSERT(diff_point.m_path.front().m_position == 0);
                    diff_point.m_path.front().m_position = position;
                    diff_point.m_path.push_front(node_data);
                    diff_points.push_back(std::move(diff_point));
                }
                ++position;
            }
            break;
        }
    }

    return diff_points;
}

std::vector<XmlDiffPoint> make_diff(const XmlDoc& old_doc, const XmlDoc& new_doc, const std::string& id)
{
    auto cloned_old_doc = xml_tools::clone_doc(old_doc, (stl_tools::flag | xml_tools::XmlCloneOpt::ALL) % xml_tools::XmlCloneOpt::PRIVATE); // because diffTree overwrites xmlNode->_private
    auto diff_doc = xml_tools::clone_doc(new_doc, (stl_tools::flag | xml_tools::XmlCloneOpt::ALL) % xml_tools::XmlCloneOpt::PRIVATE);

    std::vector<xmlstring> ids = {BAD_CAST id.c_str()};
    xmldiff_options opt = {
        false,  // doNotTagDiff
        false,  // tagChildsAddedRemoved
        true,   // beforeValue
        true,   // doNotFreeBeforeTreeItems
        false,  // diff only
        false,  // keep diff only
        true,
        false,
        false,
        (xmlChar*)"status",
        (xmlChar*)"url",
        (xmlChar*)"diff",
        (xmlChar*)"%%%",
        {(xmlChar*)"DN_UNKNOWN", (xmlChar*)"DN_ADDED", (xmlChar*)"DN_REMOVED", (xmlChar*)"DN_MODIFIED", (xmlChar*)"DN_BELOW", (xmlChar*)"DN_NONE"},
        ids,
        {},
        (xmlChar*)(""),
        NULL,
        NULL
    };

    diffTree((xmlNodePtr)cloned_old_doc.get(), (xmlNodePtr)diff_doc.get(), opt);
    return compress_diff(*get_root(*diff_doc), id);
}

}} //namespace TR { namespace Core {