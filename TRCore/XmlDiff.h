#pragma once
#include <string>
#include <vector>
#include <boost\optional.hpp>
namespace TR { namespace Core {

struct XmlNodeData
{
    XmlNodeData():
        m_position(0)
    {
    }

    XmlNodeData(std::string name, int position = 0, boost::optional<std::string> id = boost::none, boost::optional<std::string> value = boost::none):
        m_name(std::move(name)),
        m_id(std::move(id)),
        m_value(std::move(value)),
        m_position(position)
    {
    }

    bool is_attr() const
    {
        return m_name.empty() == false && m_name[0] == '@';
    }

    bool is_element() const
    {
        return m_name.empty() == false && m_name[0] != '@';
    }

    bool is_value() const
    {
        return m_name.empty();
    }

    std::string                     m_name;
    boost::optional<std::string>    m_id;
    boost::optional<std::string>    m_value;
    int                             m_position;
    int                             m_new_position;
};

inline bool operator == (const XmlNodeData& left, const XmlNodeData& right)
{
    return left.m_name == right.m_name &&
        left.m_id == right.m_id &&
        left.m_value == right.m_value &&
        left.m_position == right.m_position;
}

struct XmlDiffPoint
{
    enum State { STATE_NONE = 0, STATE_INSERTED, STATE_REMOVED };
    enum Type { ELEMENT = 0x01, TEXT = 0x02, ATTRIBUTE = 0x04, ATTR_VALUE = 0x08, ANY = 0xFF };
    using Path = std::deque<XmlNodeData>;   
    
    XmlDiffPoint():
        m_state(STATE_NONE)
    {
    }

    XmlDiffPoint(State state):
        m_state(state)
    {
    }

    Type get_type() const;

    Path m_path;
    State m_state;
};

std::vector<XmlDiffPoint> make_diff(const XML::XmlDoc& old_doc, const XML::XmlDoc& new_doc, const std::string& id);

}} //namespace TR { namespace Core {