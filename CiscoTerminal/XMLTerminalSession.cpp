#include "stdafx.h"
#include "XMLTerminalSession.h"
#include "TerminalSession.h"

namespace TerminalSession {
using namespace xml_tools;

XmlDataNode::Type XmlDataNode::get_type() const
{
    if (node_name_equal(*this, "text"))
    {
        return Type::TEXT;
    }
    else if (node_name_equal(*this, "variable"))
    {
        return Type::VARIABLE;
    }
    else
    {
        throw std::logic_error("Invalid data node type found");
    }
}

std::string XmlStaticTextNode::get_text() const
{
    return get_node_text(*this);
}

std::string XmlVariableNode::get_variable_name() const
{
    return get_node_text(*this);
}


const XmlDataNode* XmlSendNode::next_send_data(const XmlDataNode* data_node)
{
    return static_cast<const XmlDataNode*>(next_element(*data_node));
}

XmlSendNode::ConstSendDataIterator XmlSendNode::begin_send_data() const
{
    return ConstSendDataIterator(static_cast<const XmlDataNode*>(begin_element(*this)));
}

XmlSendNode::ConstSendDataIterator XmlSendNode::end_send_data() const
{
    return ConstSendDataIterator();
}

XmlSendNode::ConstSendDataIteratorRange XmlSendNode::get_send_data() const
{
    return ConstSendDataIteratorRange( begin_send_data(),  end_send_data() );
}

std::string XmlExpectedNode::get_match_pattern() const
{
    auto pattern_node = find_child(*this, "pattern");
    if (!pattern_node)
    {
        throw std::logic_error("Expected pattern not found");
    }

    return get_node_text(*pattern_node);
}

const XmlActionNode& XmlExpectedNode::get_action() const
{
    for (auto element_node = begin_element(*this); element_node; element_node = next_element(*element_node))
    {
        if (node_name_equal(*element_node, "exchange") ||
            node_name_equal(*element_node, "fail") ||
            node_name_equal(*element_node, "complete"))
        {
            return static_cast<const XmlActionNode&>(*element_node);
        }
    }

    throw std::logic_error("Expected action node not found");
}

const XmlActionNode& XmlUnexpectedNode::get_action() const
{    
    if(auto element_node = begin_element(*this))
    {
        if (node_name_equal(*element_node, "exchange") ||
            node_name_equal(*element_node, "fail") ||
            node_name_equal(*element_node, "complete"))
        {
            return static_cast<const XmlActionNode&>(*element_node);
        }
    }    
   
    throw std::logic_error("Unexpected action not found");
}

XmlActionNode::Type XmlActionNode::get_type() const
{
    if (node_name_equal(*this, "exchange"))
    {
        return Type::EXCHANGE;
    }
    else if (node_name_equal(*this, "complete"))
    {
        return Type::COMPLETE;
    }
    else if (node_name_equal(*this, "fail"))
    {
        return Type::FAIL;
    }
    else
    {
        throw std::logic_error("Invalid action node type found");
    }
}

const XmlSendNode& XmlExchangeNode::get_send_node() const
{
    auto send_node = find_child(*this, "send");
    if (!send_node)
    {
        throw std::logic_error("Send node not found");
    }

    return static_cast<const XmlSendNode&>(*send_node);
}

const XmlRecvNode& XmlExchangeNode::get_recv_node() const
{
    auto recv_node = find_child(*this, "recv");
    if (!recv_node)
    {
        throw std::logic_error("Recv node not found");
    }

    return static_cast<const XmlRecvNode&>(*recv_node);
}

const XmlExpectedNode* XmlRecvNode::next_expected_node(const XmlExpectedNode* expected_node)
{
    return static_cast<const XmlExpectedNode*>(find_sibling(*expected_node, "expected"));
}
    
XmlRecvNode::ConstExpectedNodeIterator XmlRecvNode::begin_expected_node() const
{
    return ConstExpectedNodeIterator(static_cast<const XmlExpectedNode*>(find_child(*this, "expected")));
}

XmlRecvNode::ConstExpectedNodeIterator XmlRecvNode::end_expected_node() const
{
    return ConstExpectedNodeIterator();
}

XmlRecvNode::ConstExpectedNodeIteratorRange XmlRecvNode::get_expected_nodes() const
{
    return ConstExpectedNodeIteratorRange(begin_expected_node(), end_expected_node());
}

const XmlUnexpectedNode* XmlRecvNode::get_unexpected_node() const
{    
    return static_cast<const XmlUnexpectedNode*>(find_child(*this, "unexpected"));
}

XmlCommandNode::Type XmlCommandNode::get_type() const
{
    if (node_name_equal(*this, "exec"))
    {
        return Type::EXEC;
    }
    else if (node_name_equal(*this, "mode"))
    {
        return Type::MODE;
    }
    else
    {
        throw std::logic_error("Invalid command type found");
    }
}

const XmlExchangeNode& XmlCommandNode::get_exchange_node() const
{
    auto exchange_node = find_child(*this, "exchange");
    if (!exchange_node)
    {
        throw std::logic_error("Exchange node not found");
    }

    return static_cast<const XmlExchangeNode&>(*exchange_node);
}

const XmlCommandNode* XmlCommandsNode::next_command_node(const XmlCommandNode* command_node)
{
    _ASSERT(command_node);
    return static_cast<const XmlCommandNode*>(next_element(*command_node));
}

XmlCommandsNode::ConstCommandNodeIterator XmlCommandsNode::begin_command_node() const
{
    return ConstCommandNodeIterator(static_cast<const XmlCommandNode*>(begin_element(*this)));
}

XmlCommandsNode::ConstCommandNodeIterator XmlCommandsNode::end_command_node() const
{
    return ConstCommandNodeIterator();
}

XmlCommandsNode::ConstCommandNodeIteratorRange XmlCommandsNode::get_command_nodes() const
{
    return ConstCommandNodeIteratorRange(begin_command_node(), end_command_node());
}

const XmlCommandsNode& XmlTerminalSessionDoc::get_commands() const
{
    auto root_node = get_root(*this);
    if (!root_node)
    {
        throw std::logic_error("Invalid terminal session document");
    }

    auto commands_node = find_child(*root_node, "commands");
    if (!commands_node)
    {
        throw std::logic_error("Commands node not found");
    }

    return static_cast<const XmlCommandsNode&>(*commands_node);
}

std::unique_ptr<XmlTerminalSessionDoc> parse_terminal_session(const std::string& terminal_session_xml)
{
    return stl_tools::static_pointer_cast<XmlTerminalSessionDoc>(xml_tools::parse(terminal_session_xml));
}

std::unique_ptr<XmlTerminalSessionDoc> load_terminal_session(const std::string& file_system_path)
{
    std::ifstream fs(file_system_path);
    if (!fs)
    {
        throw std::runtime_error("Can`t read file");
    }

    return parse_terminal_session({
        std::istreambuf_iterator<char>(fs),
        std::istreambuf_iterator<char>()
        });
}

static std::unique_ptr<Command> create_send_command(const XmlSendNode& send_node, const std::map<std::string, std::string>& variables)
{
    std::string send_data;
    for (auto& send_data_node : send_node.get_send_data())
    {
        switch (send_data_node.get_type())
        {
        case XmlDataNode::Type::TEXT:
        {
            auto& static_text = static_cast<const XmlStaticTextNode&>(send_data_node);
            send_data.append(static_text.get_text());
            break;
        }

        case XmlDataNode::Type::VARIABLE:
        {
            auto& variable_node = static_cast<const XmlVariableNode&>(send_data_node);
            send_data.append(variables.at(variable_node.get_variable_name()));
            break;
        }

        default:
            _ASSERT(false);            
        }
    }

    return std::make_unique<Send>(std::move(send_data));
}

static std::unique_ptr<Command> create_recv_command(const XmlRecvNode& recv_node, const std::map<std::string, std::string>& variables, bool change_prompt);

static std::unique_ptr<Command> create_exchange_command(const XmlExchangeNode& exchange_node, const std::map<std::string, std::string>& variables, bool change_prompt)
{
    auto batch = std::make_unique<Batch>();
    batch->add_command(create_send_command(exchange_node.get_send_node(), variables));
    batch->add_command(create_recv_command(exchange_node.get_recv_node(), variables, change_prompt));
    return batch;
}

static std::unique_ptr<Command> create_complete_command(const XmlCompleteNode& complete_node, const std::map<std::string, std::string>& variables, bool change_prompt)
{
    if (change_prompt)
    {
        return std::make_unique<SetPrompt>();
    }

    auto recv = std::make_unique<Recv>();
    recv->set_unexpected(std::make_unique<Print>(std::cout));
    return recv;
}

static std::unique_ptr<Command> create_fail_command(const XmlFailNode& fail_node, const std::map<std::string, std::string>& variables)
{
    auto batch = std::make_unique<Batch>();
    batch->add_command(std::make_unique<SetErrorCode>(-1));
    auto recv = std::make_unique<Recv>();
    recv->set_unexpected(std::make_unique<Print>(std::cerr));
    batch->add_command(std::move(recv));
    return batch;
}

static std::unique_ptr<Command> create_action_command(const XmlActionNode& action_node, const std::map<std::string, std::string>& variables, bool change_prompt)
{
    switch (action_node.get_type())
    {
    case XmlActionNode::Type::EXCHANGE:
        return create_exchange_command(static_cast<const XmlExchangeNode&>(action_node), variables, change_prompt);

    case XmlActionNode::Type::COMPLETE:
        return create_complete_command(static_cast<const XmlCompleteNode&>(action_node), variables, change_prompt);

    case XmlActionNode::Type::FAIL:
        return create_fail_command(static_cast<const XmlFailNode&>(action_node), variables);

    default:
        _ASSERT(false);
    }

    return nullptr;
}

static std::unique_ptr<Command> create_recv_command(const XmlRecvNode& recv_node, const std::map<std::string, std::string>& variables, bool change_prompt)
{
    auto recv = std::make_unique<Recv>();
    for (auto& expected_node : recv_node.get_expected_nodes())
    {
        recv->add_expected({
            std::make_unique<RegexCondition>(expected_node.get_match_pattern()),
            create_action_command(expected_node.get_action(), variables, change_prompt)
        });        
    }

    if (auto unexpected_node = recv_node.get_unexpected_node())
    {
        recv->set_unexpected(
            create_action_command(unexpected_node->get_action(), variables, change_prompt)
        );
    }

    return recv;
}

std::unique_ptr<Command> create_mode_command(const XmlModeNode& mode_node, const std::map<std::string, std::string>& variables)
{
    return create_exchange_command(mode_node.get_exchange_node(), variables, true);
}

std::unique_ptr<Command> create_exec(const XmlExecNode& exec_node, const std::map<std::string, std::string>& variables)
{
    return create_exchange_command(exec_node.get_exchange_node(), variables, false);
}

std::unique_ptr<Command> create_command(const XmlCommandNode& command_node, const std::map<std::string, std::string>& variables)
{
    switch(command_node.get_type())
    {
    case XmlCommandNode::Type::MODE:
        return create_mode_command(static_cast<const XmlModeNode&>(command_node), variables);

    case XmlCommandNode::Type::EXEC:
        return create_exec(static_cast<const XmlExecNode&>(command_node), variables);

    default:
        _ASSERT(false);
    }

    return nullptr;
}

std::vector<std::unique_ptr<Command>> create_terminal_session(const XmlTerminalSessionDoc& terminal_session_doc, const std::map<std::string, std::string>& variables)
{
    std::vector<std::unique_ptr<Command>> commands;
    auto& commands_node = terminal_session_doc.get_commands();
    for (auto& command_node : commands_node.get_command_nodes())
    {
        commands.push_back(create_command(command_node, variables));
    }

    return commands;
}

} //namespace TerminalSession {