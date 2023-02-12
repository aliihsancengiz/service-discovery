#pragma once

#include "connection_config.hpp"
#include "defines.hpp"

#include <boost/bind.hpp>

struct IPacketObserver
{
    virtual void process_packet(const std::string& buffer) = 0;
};

struct DiscoverySocket
{
    DiscoverySocket(boost::asio::io_service& io, const connection_config& cfg)
        : io_service(io), socket(io), _cfg(cfg)
    {
        socket.open(boost::asio::ip::udp::v4());
        socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket.bind(_cfg.listen_addr());
        socket.set_option(
          boost::asio::ip::multicast::join_group(_cfg.multicast_address().to_v4(), {}));
        _packet_observer = nullptr;
        start_receive();
    }

    void set_packet_observer(IPacketObserver* packet_observer)
    {
        _packet_observer = packet_observer;
    }

    void reset_packet_observer()
    {
        _packet_observer = nullptr;
    }

  private:
    void start_receive()
    {
        recv_buffer_str.resize(SERVICE_RECEIVE_BUFFER_LENGTH);
        socket.async_receive(boost::asio::buffer(recv_buffer_str),
                             boost::bind(&DiscoverySocket::handle_receive, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
    }
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        if (error) {
            return;
        }
        if (_packet_observer) {
            _packet_observer->process_packet(recv_buffer_str);
        }

        recv_buffer_str.clear();
        start_receive();
    }
    boost::asio::io_service& io_service;
    boost::asio::ip::udp::socket socket;
    const connection_config& _cfg;
    std::string recv_buffer_str;
    IPacketObserver* _packet_observer;
};
