#pragma once

#include "connection_config.hpp"
#include "defines.hpp"
#include "service_message.hpp"

#include <boost/array.hpp>
#include <boost/bind.hpp>

namespace service_announcment {

namespace detail {

    struct ServiceAnnouncementSocket
    {

        ServiceAnnouncementSocket(boost::asio::io_service& io, const connection_config& cfg)
            : io_service(io), socket(io), _cfg(cfg)
        {
            socket.open(boost::asio::ip::udp::v4());
        }
        ~ServiceAnnouncementSocket()
        {
            socket.close();
        }

        void send_announcement(const std::string& message_str)
        {
            socket.send_to(boost::asio::buffer(message_str), _cfg.announce_addr());
        }

      private:
        boost::asio::io_service& io_service;
        boost::asio::ip::udp::socket socket;
        const connection_config& _cfg;
    };

}  // namespace detail

struct ServiceAnnouncement
{
    ServiceAnnouncement(boost::asio::io_service& io, const connection_config& cfg)
        : io_service(io), _socket(io, cfg),
          announcement_timer(io, boost::asio::chrono::milliseconds(SERVICE_ANNOUCMENT_TIMER_PERIOD))
    {
    }
    void start_announce(const std::vector<service_message>& messages)
    {
        _services = messages;
        announcement_timer.async_wait(
          boost::bind(&ServiceAnnouncement::announce, this, boost::asio::placeholders::error));
    }

  private:
    boost::asio::io_service& io_service;
    service_announcment::detail::ServiceAnnouncementSocket _socket;
    boost::asio::steady_timer announcement_timer;
    std::vector<service_message> _services;

    void announce(const boost::system::error_code& /*e*/)
    {
        static uint64_t counter{0};
        for (auto service : _services) {
            if (((counter * 1000) % service.ttl_msec()) == 0) {
                _socket.send_announcement(message_serdes::to_json(service));
            }
        }
        announcement_timer.expires_at(
          announcement_timer.expiry() +
          boost::asio::chrono::milliseconds(SERVICE_ANNOUCMENT_TIMER_PERIOD));
        announcement_timer.async_wait(
          boost::bind(&ServiceAnnouncement::announce, this, boost::asio::placeholders::error));
        counter++;
    }
};

}  // namespace service_announcment
