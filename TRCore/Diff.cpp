#include "stdafx.h"
#include "Diff.h"
#include <boost/algorithm/string.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/optional.hpp>
#include <dtl/dtl.hpp>
namespace TR { namespace Core {

template<class Archive>
inline void serialize(Archive &archive, DiffLine& diff_line, const unsigned int /*version*/)
{
    archive & diff_line.m_diff_state;
    archive & diff_line.m_line_pos;
    archive & diff_line.m_line_str;
}

PlainTextDiff::PlainTextDiff(Blob blob)
{
    stl_tools::read_blob(blob, m_diff_lines);
}

PlainTextDiff::PlainTextDiff(DiffLines diff_lines):
    m_diff_lines(std::move(diff_lines))
{
}

Blob PlainTextDiff::as_blob() const
{
    return stl_tools::make_blob(m_diff_lines);
}

const PlainTextDiff::DiffLines& PlainTextDiff::get_diff_lines() const
{
    return m_diff_lines;
}

template<class Archive>
inline void serialize(Archive &archive, XmlNodeData& xml_node_data, const unsigned int /*version*/)
{   
    archive & xml_node_data.m_name;
    archive & xml_node_data.m_id;
    archive & xml_node_data.m_value;
    archive & xml_node_data.m_position;
}

template<class Archive>
inline void serialize(Archive &archive, XmlDiffPoint& xml_diff_point, const unsigned int /*version*/)
{
    archive & xml_diff_point.m_path;
    archive & xml_diff_point.m_state;
}

PlainXmlDiff::PlainXmlDiff(Blob blob)
{
    stl_tools::read_blob(blob, m_diff_points);
}

PlainXmlDiff::PlainXmlDiff(DiffPoints diff_points):
    m_diff_points(std::move(diff_points))
{
}

Blob PlainXmlDiff::as_blob() const
{
    return stl_tools::make_blob(m_diff_points);
}

const PlainXmlDiff::DiffPoints& PlainXmlDiff::get_diff_points() const
{
    return m_diff_points;
}

RegularDiff::RegularDiff(Blob blob, std::shared_ptr<const XmlDefDoc> def_doc):
    PlainXmlDiff(std::move(blob)),
    m_def_doc(def_doc)
{
}

RegularDiff::RegularDiff(DiffPoints diff_points, std::shared_ptr<const XmlDefDoc> def_doc):
    PlainXmlDiff(std::move(diff_points)),
    m_def_doc(def_doc)
{
}

std::wstring RegularDiff::format_path(size_t diff_point_pos) const
{
    auto& diff_point = m_diff_points.at(diff_point_pos);
    const XML::XmlDefNode* def_node = (const XML::XmlDefNode*)(m_def_doc.get());
    
    std::wstring path;
    for (auto& node_data : diff_point.m_path)
    {
        if (node_data.m_value)
        {
            path.append(L" = ");
            path.append(stl_tools::utf8_to_ucs(*node_data.m_value));
            break;
        }

        def_node = static_cast<const XML::XmlDefNode*>(xml_tools::find_child(*def_node, node_data.m_name.c_str()));
        if (!def_node)
        {
            throw Exception(L"Invalid diff format");
        }

        if (!path.empty())
        {
            path.append(L" / ");
        }

        path.append(def_node->get_caption());

        if (node_data.m_id)
        {
            path.append(L":");
            path.append(stl_tools::utf8_to_ucs(*node_data.m_id));
        }       
        
    }
    return path;
}

}} //namespace TR { namespace Core {