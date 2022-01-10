#include "stdafx.h"
#include "TerminalSession.h"

namespace TerminalSession {

namespace {
char* recv_start(Session& session)
{
    return &session.m_buffer[session.m_start];
}

char* recv_stop(Session& session)
{
    return &session.m_buffer[session.m_stop];
}

unsigned recv_count(Session& session)
{
    _ASSERT(session.m_stop >= session.m_start);
    return session.m_stop - session.m_start;
}

unsigned buffer_size(Session& session)
{
    return sizeof(session.m_buffer) - 1;
}

unsigned buffer_pos(Session& session, char* pointer)
{
    _ASSERT(pointer >= recv_start(session) && pointer <= recv_stop(session));
    return unsigned(pointer - session.m_buffer);
}

char* trim_front(char* begin, char* end)
{
    auto current = begin;
    for (; current < end && isspace(*current); ++current);
    return current;
}

char* trim_back(char* begin, char* end)
{
    auto current = end;
    for (; current > begin && isspace(*(current - 1)); --current);
    return current;
}

void trim_front(Session& session)
{
    auto begin = trim_front(recv_start(session), recv_stop(session));
    session.m_start = buffer_pos(session, begin);
}

void trim_back(Session& session)
{
    auto end = trim_back(recv_start(session), recv_stop(session));
    session.m_stop = buffer_pos(session, end);
}

void ssh_check_error(Session& session, const char* message = nullptr)
{
    const char* error_string = nullptr;
    if (session.m_channel)
    {
        auto ssh_session = ssh_channel_get_session(session.m_channel);
        if (ssh_get_error_code(ssh_session) == SSH_NO_ERROR)
        {
            return;
        }

        error_string = ssh_get_error(ssh_session);
        if (error_string)
        {
            error_string = "Unknown ssh error";
        }
    }
    else
    {
        error_string = "Failed to create SSH channel";
    }

    if (message)
    {
        std::cerr << message << " : ";
    }

    std::cerr << error_string << std::endl;
    exit(-1);
}

std::ostream& write(std::ostream& out, char* buffer, size_t count)
{   
    for (size_t i = 0; out && (i < count); ++i)
    {
        auto ch = buffer[i];
        if (ch != '\r')
        {
            out.put(ch);
        }        
    }
    return out;
}

}

Batch::Batch(std::vector<std::unique_ptr<Command>> commands) :
    m_commands(std::move(commands))
{
}

void Batch::add_command(std::unique_ptr<Command> command)
{
    m_commands.push_back(std::move(command));
}

void Batch::run(Session& session)
{
    for (auto& command : m_commands)
    {
        command->run(session);
    }
}

RegexCondition::RegexCondition(std::string regex) :
    m_regex(std::move(regex))
{
}

bool RegexCondition::match(Session& session)
{
    return boost::regex_search(
        &session.m_buffer[session.m_start],
        &session.m_buffer[session.m_stop],
        m_regex);
}

Send::Send(std::string what):
    m_what(what)
{
}

bool skip(Session& session, const char* what, unsigned what_size)
{
    for (;;)
    {
        trim_front(session);

        if (recv_count(session) >= what_size)
        {
            if (std::strncmp(what, recv_start(session), what_size) == 0)
            {
                session.m_start += what_size;
                trim_front(session);
                return true;
            }
            return false;
        }

        auto count = ssh_channel_read_timeout(session.m_channel, session.m_buffer + session.m_stop, buffer_size(session) - session.m_stop, 0, 1000);
        ssh_check_error(session, "Terminal skip failed");

        if (!count)
        {
            return false;
        }

        //std::cout.write(session.m_buffer, count);
        //std::cout.flush();

        session.m_stop += count;
    }

    return false;
}

void Send::run(Session& session)
{
    ssh_channel_write(session.m_channel, m_what.c_str(), uint32_t(m_what.size()));
    ssh_check_error(session, "Terminal send failed");

    ssh_channel_write(session.m_channel, "\n", 1);
    ssh_check_error(session, "Terminal send failed");

    session.m_start = session.m_stop = 0;
    session.m_bof = true;

    if (session.m_eof)
    {
        if (!skip(session, m_what.c_str(), unsigned(m_what.size())))
        {
            std::cerr << "Failed to skip echo" << std::endl;
            exit(-1);
        }       
    }

    session.m_eof = false;
}

Print::Print(std::ostream& out):
    m_out(out)
{    
}

void Print::run(Session& session)
{
    if (!write(m_out, recv_start(session), recv_count(session)))
    {
        std::cerr << "Failed to write" << std::endl;
        exit(-1);
    }

    session.m_start = session.m_stop = 0;
}

SetPrompt::SetPrompt()
{
}

void SetPrompt::run(Session& session)
{
    if (session.m_eof)
    {
        session.m_start = session.m_stop = 0;
        return;
    }

    auto start = std::find_if(
        std::make_reverse_iterator(recv_stop(session)),
        std::make_reverse_iterator(recv_start(session)),
        (int (*)(int)) (&std::isspace)).base();

    session.m_prompt = std::string(start, recv_stop(session));
    session.m_eof = true;
    session.m_start = session.m_stop = 0;
}

SetErrorCode::SetErrorCode(int error_code):
    m_error_code(error_code)
{
}

void SetErrorCode::run(Session& session)
{
    session.m_error_code = m_error_code;
}

Recv::Recv() = default;

void Recv::add_expected(Expected expected)
{
    m_expected.push_back(std::move(expected));
}

void Recv::set_unexpected(std::unique_ptr<Command> command)
{
    m_unexpected = std::move(command);
}

const Expected* find_expected(Session& session, const std::vector<Expected>& expected)
{
    for (auto& e : expected)
    {
        if (e.m_condition->match(session))
        {
            return &e;
        }
    }
    return nullptr;
}

void Recv::run(Session& session)
{    
    for (;;)
    {
        if (session.m_eof && recv_count(session) == 0)
        {
            return;
        }
        
        if (recv_count(session) == 0)
        {
            auto count = ssh_channel_read(session.m_channel, session.m_buffer, buffer_size(session), 0);
            ssh_check_error(session, "Failed to receive data");

            //std::cout.write(session.m_buffer, count);
            //std::cout.flush();

            session.m_start = 0;
            session.m_stop = count;            
        }

        if (session.m_bof)
        {
            trim_front(session);
            session.m_bof = (recv_count(session) == 0);
        }

        if (recv_count(session) == 0)
        {
            continue;
        }

        if (auto prompt_size = session.m_prompt.size())
        {
            if (recv_count(session) >= prompt_size)
            {
                if (session.m_prompt.compare(0, prompt_size, recv_stop(session) - prompt_size, prompt_size) == 0)
                {
                    session.m_eof = true;
                    session.m_stop -= unsigned(prompt_size);
                    //trim_back(session);
                }
            }
        }

        if (auto expected = find_expected(session, m_expected))
        {
            if (auto& command = expected->m_command)
            {
                command->run(session);
            }
            else
            {
                session.m_start = session.m_stop = 0;
            }

            if (session.m_break)
            {
                session.m_break = false;
                break;
            }
            else
            {
                continue;
            }
        }

        if (!session.m_eof)
        {
            auto count = ssh_channel_read_timeout(session.m_channel, recv_stop(session), buffer_size(session) - session.m_stop, 0, 10000);
            ssh_check_error(session, "Failed to receive data");
            
            if (count)
            {
                session.m_stop += count;
                continue;
            }
        }

        if (m_unexpected)
        {
            m_unexpected->run(session);
            if (session.m_break)
            {
                session.m_break = false;
                break;
            }
        }
        else
        {
            break;
        }        
    }
}

void run(Session& session, std::vector<std::unique_ptr<Command>>& commands)
{
    for (auto& command : commands)
    {
        command->run(session);
        
        if (session.m_error_code != 0)
        {
            exit(session.m_error_code);
        }

        if (!session.m_eof)
        {
            exit(-1);
        }
    }
}

} //namespace TerminalSession {