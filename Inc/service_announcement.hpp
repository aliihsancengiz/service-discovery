#pragma once

#include "connection_config.hpp"
#include "defines.hpp"
#include "service_message.hpp"

#include <boost/array.hpp>
#include <boost/bind.hpp>

struct service_announcement
{
    service_announcement(boost::asio::io_service& io, const connection_config& cfg)
        : io_service(io), socket(io), _cfg(cfg),
          announcement_timer(io, boost::asio::chrono::milliseconds(SERVICE_ANNOUCMENT_TIMER_PERIOD))
    {
        socket.open(boost::asio::ip::udp::v4());
    }
    void start_announce(const std::vector<service_message>& messages)
    {
        _services = messages;
        announcement_timer.async_wait(
          boost::bind(&service_announcement::announce, this, boost::asio::placeholders::error));
    }

  private:
    boost::asio::io_service& io_service;
    boost::asio::ip::udp::socket socket;
    const connection_config& _cfg;
    boost::array<char, 1024> send_buf;
    boost::asio::steady_timer announcement_timer;
    std::vector<service_message> _services;

    void announce(const boost::system::error_code& /*e*/)
    {
        static uint64_t counter{0};
        for (auto service : _services) {
            if (((counter * 1000) % service.ttl_msec()) == 0) {
                boost::array<char, SERVICE_SEND_BUFFER_LENGTH> send_buf = {{0}};
                std::string message_str = message_serdes::to_json(service);
                std::copy(message_str.begin(), message_str.end(), send_buf.begin());

                socket.send_to(boost::asio::buffer(send_buf), _cfg.announce_addr());
            }
        }
        announcement_timer.expires_at(
          announcement_timer.expiry() +
          boost::asio::chrono::milliseconds(SERVICE_ANNOUCMENT_TIMER_PERIOD));
        announcement_timer.async_wait(
          boost::bind(&service_announcement::announce, this, boost::asio::placeholders::error));
        counter++;
    }
};
