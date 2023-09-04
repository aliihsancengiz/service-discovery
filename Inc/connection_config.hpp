#pragma once

#include <boost/asio.hpp>
#include <string>

struct connection_config
{
    connection_config(const std::string ip, const std::size_t port)
        : multicast_addr(boost::asio::ip::address::from_string(ip)),
          listen_ep(boost::asio::ip::udp::v4(), port),
          announce_ep(boost::asio::ip::address::from_string(ip), port)
    {
    }
    ~connection_config() = default;

    auto& multicast_address() const
    {
        return multicast_addr;
    }
    auto& listen_addr() const
    {
        return listen_ep;
    }
    auto& announce_addr() const
    {
        return announce_ep;
    }

  private:
    boost::asio::ip::address multicast_addr;
    boost::asio::ip::udp::endpoint listen_ep;
    boost::asio::ip::udp::endpoint announce_ep;
};