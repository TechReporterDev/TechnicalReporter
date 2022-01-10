#pragma once
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <boost/variant.hpp>
#include <boost/regex.hpp>
#include <libssh/libssh.h>

namespace TerminalSession {

struct Session
{
    ssh_channel             m_channel = nullptr;

    char                    m_buffer[1000] = {0};
    unsigned                m_start = 0;
    unsigned                m_stop = 0;

    bool                    m_bof = true;
    bool                    m_eof = false;
    bool                    m_break = false;
    int                     m_error_code = 0;

    std::string             m_prompt;
};

class Command
{
public:
    virtual void run(Session& session) = 0;
};

class Condition
{
public:
    virtual bool match(Session& session) = 0;
};

struct Expected
{
    std::unique_ptr<Condition> m_condition;
    std::unique_ptr<Command> m_command;
};

class Batch : public Command
{
public:
    Batch() = default;
    Batch(std::vector<std::unique_ptr<Command>> commands);
    void add_command(std::unique_ptr<Command> command);

    // Command override
    virtual void run(Session& session);

private:
    std::vector<std::unique_ptr<Command>> m_commands;
};

class RegexCondition : public Condition
{
public:
    RegexCondition(std::string regex);

    // Condition override
    virtual bool match(Session& session);

private:
    boost::regex m_regex;
};

class Send : public Command
{
public:
    Send(std::string what);

    // Command override
    virtual void run(Session& session);

private:
    std::string m_what;
};

class Print : public Command
{
public:
    Print(std::ostream& out);

    // Command override
    virtual void run(Session& session);

private:
    std::ostream& m_out;
};

class SetPrompt : public Command
{
public:
    SetPrompt();

    // Command override
    virtual void run(Session& session);
};

class SetErrorCode : public Command
{
public:
    SetErrorCode(int error_code);

    // Command override
    virtual void run(Session& session);

private:
    int m_error_code;
};

class Recv : public Command
{
public:
    Recv();
    void add_expected(Expected expected);
    void set_unexpected(std::unique_ptr<Command> command);

    // Command override
    virtual void run(Session& session);   

private:
    std::vector<Expected> m_expected;
    std::unique_ptr<Command> m_unexpected;
};

void run(Session& session, std::vector<std::unique_ptr<Command>>& commands);

} //namespace TerminalSession 