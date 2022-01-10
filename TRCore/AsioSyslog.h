/////////////
//From http://grayhole.blogspot.ru/2008/10/boostasio-syslog.html
/////////////

#pragma once
#include <thread>
#include <boost/asio.hpp>
namespace TR { namespace Core {

class AsioSyslog
{
public:
    AsioSyslog(short port = 514);
    ~AsioSyslog();

    template<class Slot>
    void connect(Slot slot)
    {
        m_signal.connect(std::move(slot));
    }

private:
    void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd);
    void invoke_recieve();

private:
    boost::signals2::signal<void(const std::string& address, const std::string& log)> m_signal;

    char m_buffer[4096];
    boost::asio::io_service m_io_service;
    boost::asio::ip::udp::socket m_socket;
    boost::asio::ip::udp::endpoint m_sender_endpoint;
    std::thread m_thread;
};

}} //namespace TR { namespace Core {