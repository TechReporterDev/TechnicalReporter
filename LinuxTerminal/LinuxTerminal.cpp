#include "stdafx.h"
#include "boost\program_options.hpp"
#include "boost\regex.hpp"
#include <boost\algorithm\string.hpp>

namespace po = boost::program_options;

char* trim_last(char* begin, char* end)
{
    auto current = end;
    for (; current > begin && isspace(*(current-1)); --current)
    {       
    }
    return current;
}

std::string get_setting(xml_tools::XmlDoc& settings, const char* xpath)
{
    auto setting = xml_tools::find_xpath_node(settings, xpath);
    if (!setting)
    {
        exit(-1);       
    }
    return xml_tools::get_node_text(*setting);  
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

int _tmain(int argc, _TCHAR* argv[])
{
    //::Sleep(20000);
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("command", po::value<std::string>(), "command")
        ("address", po::value<std::string>(), "IP address")
        ("login", po::value<std::string>(), "login")
        ("password", po::value<std::string>(), "password")
        ("question", po::value<std::vector<std::string>>(), "question")
        ("answer", po::value<std::vector<std::string>>(), "answer")
        ("break", po::value<std::vector<std::string>>(), "break");

    if (argc < 2)
    {
        desc.print(std::cout);
        return 0;
    }

    po::positional_options_description p;
    p.add("command", -1);
    
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
        options(desc).positional(p).run(), vm);
    po::notify(vm);
    
    if (vm.count("help"))
    {
        desc.print(std::cout);
        return 0;
    }

    const std::string address = vm["address"].as<std::string>();
    const std::string login = vm["login"].as<std::string>();
    const std::string password = vm["password"].as<std::string>();
    const std::string cmd = vm["command"].as<std::string>();

    std::vector<boost::regex> questions;
    if (vm.count("question"))
    {
        for (auto& question : vm["question"].as<std::vector<std::string>>())
        {
            questions.emplace_back(question);
        }
    }

    std::vector<std::string> answers;
    if (vm.count("answer"))
    {
        answers = vm["answer"].as<std::vector<std::string>>();
    }

    if (questions.size() != answers.size())
    {
        std::cout << "Count of question is not equal to count of answer";
        exit(-1);
    }   

    std::vector<boost::regex> breaks;
    if (vm.count("break"))
    {
        for (auto& _break : vm["question"].as<std::vector<std::string>>())
        {
            breaks.emplace_back(_break);
        }
    }

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
    }*/

    rc = ssh_channel_request_exec(channel, cmd.c_str());
    if (rc != SSH_OK)
    {       
        ssh_channel_close(channel);
        ssh_channel_free(channel);   exit(-1);
    }

    char buffer[1000];

    for (size_t question = 0; question < questions.size();)
    {
        auto count = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
        if (count == SSH_ERROR)
        {
            std::cerr << "SSH channel read error: ";
            exit(-1);
        }

        if (!count)
        {
            while (auto count = ssh_channel_read(channel, buffer, sizeof(buffer), 1))
            {
                if (count == SSH_ERROR)
                {
                    std::cerr << "SSH channel read error: ";
                    exit(-1);
                }
                write(std::cerr, buffer, count);
            }
            exit(-1);
        }

        auto last = trim_last(buffer, buffer + count);
        if (buffer == last)
        {
            //ignore spaces only output
            continue;
        }

        if (!boost::regex_match(buffer, last, questions[question]))
        {
            std::cerr << "Unexpected question: ";
            write(std::cerr, buffer, count);
            exit(-1);
        }

        if (ssh_channel_write(channel, answers[question].c_str(), answers[question].size()) == SSH_ERROR)
        {
            std::cerr << "SSH channel write error: ";
            exit(-1);
        }

        if (ssh_channel_write(channel, "\n", 1) == SSH_ERROR)
        {
            std::cerr << "SSH channel write error: ";
            exit(-1);
        }

        ++question;
    }

    while (auto count = ssh_channel_read(channel, buffer, sizeof(buffer), 0))
    {
        if (count == SSH_ERROR)
        {
            exit(-1);
        }

        for (auto& _break : breaks)
        {
            if (boost::regex_match(buffer, trim_last(buffer, buffer + count), _break))
            {
                std::cerr << "Output contains break: ";
                write(std::cerr, buffer, count);
                exit(-1);
            }
        }

        write(std::cout, buffer, count);
    }

    auto exit_status = ssh_channel_get_exit_status(channel);
    if (exit_status != SSH_OK)
    {
        while (auto count = ssh_channel_read(channel, buffer, sizeof(buffer), 1))
        {
            if (count == SSH_ERROR)
            {
                exit(-1);
            }

            write(std::cerr, buffer, count);
        }
    }

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return exit_status;
}

