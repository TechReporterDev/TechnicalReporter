#pragma once
#include <vector>
#include "STLTools\stl_tools.h"
#include "XMLTools\xml_tools.h"

namespace TerminalSession {

using xml_tools::XmlNode;
using xml_tools::XmlDoc;

class Command;

struct XmlDataNode : XmlNode
{
    enum class Type { TEXT, VARIABLE };
    Type get_type() const;
};

struct XmlStaticTextNode : XmlDataNode
{
    std::string get_text() const;
};

struct XmlVariableNode : XmlDataNode
{
    std::string get_variable_name() const;
};


struct XmlSendNode : XmlNode
{
    static const XmlDataNode* next_send_data(const XmlDataNode* data_node);
    using ConstSendDataIterator = xml_tools::node_iterator<const XmlDataNode, next_send_data>;
    using ConstSendDataIteratorRange = boost::iterator_range<ConstSendDataIterator>;

    ConstSendDataIterator               begin_send_data() const;
    ConstSendDataIterator               end_send_data() const;
    ConstSendDataIteratorRange          get_send_data() const;
};

struct XmlActionNode;

struct XmlExpectedNode : XmlNode
{
    std::string             get_match_pattern() const;
    const XmlActionNode&    get_action() const;
};

struct XmlUnexpectedNode : XmlNode
{
    const XmlActionNode& get_action() const;
};

struct XmlActionNode : XmlNode
{
    enum class Type { EXCHANGE, FAIL, COMPLETE };
    Type get_type() const;
};

struct XmlRecvNode;

struct XmlExchangeNode : XmlActionNode
{
    const XmlSendNode&      get_send_node() const;
    const XmlRecvNode&      get_recv_node() const;
};

struct XmlFailNode : XmlActionNode
{
};

struct XmlCompleteNode : XmlActionNode
{
};

struct XmlRecvNode : XmlNode
{
    static const XmlExpectedNode* next_expected_node(const XmlExpectedNode* expected_node);
    using ConstExpectedNodeIterator = xml_tools::node_iterator<const XmlExpectedNode, next_expected_node>;
    using ConstExpectedNodeIteratorRange = boost::iterator_range<ConstExpectedNodeIterator>;

    ConstExpectedNodeIterator               begin_expected_node() const;
    ConstExpectedNodeIterator               end_expected_node() const;
    ConstExpectedNodeIteratorRange          get_expected_nodes() const;

    const XmlUnexpectedNode*                get_unexpected_node() const;
};

struct XmlCommandNode : XmlNode
{
    enum class Type { EXEC, MODE };
    Type get_type() const;

    const XmlExchangeNode& get_exchange_node() const;
};

struct XmlExecNode : XmlCommandNode
{
};

struct XmlModeNode : XmlCommandNode
{
};

struct XmlCommandsNode : XmlNode
{
    static const XmlCommandNode* next_command_node(const XmlCommandNode* expected_node);
    using ConstCommandNodeIterator = xml_tools::node_iterator<const XmlCommandNode, next_command_node>;
    using ConstCommandNodeIteratorRange = boost::iterator_range<ConstCommandNodeIterator>;

    ConstCommandNodeIterator               begin_command_node() const;
    ConstCommandNodeIterator               end_command_node() const;
    ConstCommandNodeIteratorRange          get_command_nodes() const;
};

struct XmlTerminalSessionDoc : XmlDoc
{
    const XmlCommandsNode& get_commands() const;    
};

std::unique_ptr<XmlTerminalSessionDoc> parse_terminal_session(const std::string& terminal_session_xml);
std::unique_ptr<XmlTerminalSessionDoc> load_terminal_session(const std::string& file_system_path);
std::vector<std::unique_ptr<Command>> create_terminal_session(const XmlTerminalSessionDoc& terminal_session_doc, const std::map<std::string, std::string>& variables);

} //namespace TerminalSession {