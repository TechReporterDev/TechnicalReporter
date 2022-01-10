#include "stdafx.h"
#include "TerminalSession.h"
#include "XMLTerminalSession.h"
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

namespace po = boost::program_options;

int _tmain(int argc, _TCHAR* argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("address", po::value<std::string>(), "IP address")
        ("login", po::value<std::string>(), "login")
        ("password", po::value<std::string>(), "password")
        ("script", po::value<std::string>(), "terminal session file path");
        

    if (argc < 2)
    {
        desc.print(std::cout);
        return 0;
    }

    auto parsed_options = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    po::variables_map vm;
    po::store(parsed_options, vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        desc.print(std::cout);
        return 0;
    }

    const std::string address = vm["address"].as<std::string>();
    const std::string login = vm["login"].as<std::string>();
    const std::string password = vm["password"].as<std::string>();
    const std::string script = vm["script"].as<std::string>();
    
    std::map<std::string, std::string> variables;
    for (auto& option : parsed_options.options)
    {
        if (option.unregistered)
        {
            variables[option.string_key] = option.value.at(0);
        }
    }

    auto terminal_session_doc = TerminalSession::load_terminal_session(script);
    auto terminal_session = create_terminal_session(*terminal_session_doc, variables);

    ssh_session my_ssh_session = ssh_new();
    if (my_ssh_session == NULL)
        exit(-1);

    int verbosity = SSH_LOG_PROTOCOL;
    int timeout = 30;

    auto s = ssh_get_kex_algo(my_ssh_session);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, address.c_str());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, login.c_str());
    //ssh_options_set(my_ssh_session, SSH_OPTIONS_TIMEOUT, &timeout);

    auto rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to localhost: %s\n", ssh_get_error(my_ssh_session));
        exit(-1);
    }

    rc = ssh_userauth_password(my_ssh_session, NULL, password.c_str());
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to localhost: %s\n", ssh_get_error(my_ssh_session));
        exit(-1);
    }

    auto channel = ssh_channel_new(my_ssh_session);
    if (channel == NULL)
    {
        exit(-1);
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK)
    {
        ssh_channel_free(channel);
        exit(-1);
    }

    /*rc = ssh_channel_request_pty(channel);
    if (rc != SSH_OK)
    {
        ssh_channel_free(channel);
        exit(-1);
    }

    rc = ssh_channel_change_pty_size(channel, 80, 24);
    if (rc != SSH_OK)
    {
        ssh_channel_free(channel);
        exit(-1);
    }*/

    rc = ssh_channel_request_shell(channel);
    if (rc != SSH_OK)
    {
        ssh_channel_free(channel);
        exit(-1);
    }

    TerminalSession::Session session { 
        channel
    };
   
    TerminalSession::run(session, terminal_session);

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return 0;
}