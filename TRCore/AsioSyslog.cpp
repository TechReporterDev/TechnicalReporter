// syslog daemon sample
#include "stdafx.h"
#include "AsioSyslog.h"
namespace TR { namespace Core {

AsioSyslog::AsioSyslog(short port):
    m_socket(m_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string("0.0.0.0"), port))
{
    invoke_recieve();
    m_thread = std::thread([this]{
        m_io_service.run();
    });
}

AsioSyslog::~AsioSyslog()
{
    m_io_service.stop();
    m_thread.join();
}

void AsioSyslog::handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
{
    if (error)
    {
        return;
    }
    
    if (bytes_recvd > 0)
    {       
        m_signal(m_sender_endpoint.address().to_string(), {m_buffer, bytes_recvd});
    }
    invoke_recieve();   
}

void AsioSyslog::invoke_recieve()
{
    m_socket.async_receive_from(
        boost::asio::buffer(m_buffer, _countof(m_buffer)),
        m_sender_endpoint,
        boost::bind(
            &AsioSyslog::handle_receive_from,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}

}} //namespace TR { namespace Core {