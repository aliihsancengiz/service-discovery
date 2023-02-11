#pragma once
#include "cache.hpp"
#include "connection_config.hpp"
#include "defines.hpp"
#include "service_message.hpp"

#include <boost/array.hpp>
#include <boost/bind.hpp>

enum class DiscoveryEvent
{
    BIRTH,
    HEARTBEAT,
    DEATH
};

struct IServiceDiscovery
{
    typedef std::function<bool(const service_message&)> service_filter;
    IServiceDiscovery(const service_filter& filter) : _filter(filter) {}
    IServiceDiscovery()
        : _filter([](const service_message&) {
              return true;
          })
    {
    }
    virtual void onNewServiceDiscovered(const service_message& new_service){};
    virtual void onServiceGoodBye(const service_message& msg){};
    virtual void onHeartBeat(const service_message& msg){};

    bool apply_filter(const service_message& msg)
    {
        return _filter(msg);
    }

  private:
    service_filter _filter;
};

struct service_discovery
{
    service_discovery(boost::asio::io_service& io, const connection_config& cfg)
        : io_service(io), socket(io), _cfg(cfg),
          cache_timer(io, boost::asio::chrono::milliseconds(SERVICE_DISCOVERY_CACHE_PRUNE_PERIOD))
    {
        socket.open(boost::asio::ip::udp::v4());
        socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket.bind(_cfg.listen_addr());
        socket.set_option(
          boost::asio::ip::multicast::join_group(_cfg.multicast_address().to_v4(), {}));

        start_receive();
        cache_timer.async_wait(
          boost::bind(&service_discovery::prune_cache, this, boost::asio::placeholders::error));
    }

    void register_observer(const std::shared_ptr<IServiceDiscovery>& cb_obj)
    {
        observerMap.push_back(cb_obj);
    }

  private:
    std::vector<std::shared_ptr<IServiceDiscovery>> observerMap;
    boost::asio::io_service& io_service;
    boost::asio::ip::udp::socket socket;
    const connection_config& _cfg;
    Cache<std::string, service_message, time_based_expiry_policy<service_message>> service_cache;
    boost::array<char, 1024> recv_buf;
    boost::asio::steady_timer cache_timer;

    void start_receive()
    {
        socket.async_receive(boost::asio::buffer(recv_buf),
                             boost::bind(&service_discovery::handle_receive, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        if (error) {
            return;
        }

        auto msg_res = message_serdes::from_json<service_message>(std::string(recv_buf.data()));
        if (msg_res.is_some()) {
            auto msg = msg_res.unwrap();
            auto cache_key = msg.uuid;

            if (service_cache.has(cache_key)) {
                msg.update_last_accessed();
                service_cache.update(cache_key, msg);
                notify_observer(msg, DiscoveryEvent::HEARTBEAT);
            } else {
                // New service discovered, emplace it to cache and notify observers
                msg.update_last_accessed();
                service_cache.add(cache_key, msg);
                notify_observer(msg, DiscoveryEvent::BIRTH);
            }
        }
        recv_buf.assign(0);
        start_receive();
    }

    void prune_cache(const boost::system::error_code& /*e*/)
    {
        for (const auto& entry : service_cache.prune_expired_entries()) {
            notify_observer(entry, DiscoveryEvent::DEATH);
        }
        cache_timer.expires_at(cache_timer.expiry() + boost::asio::chrono::milliseconds(
                                                        SERVICE_DISCOVERY_CACHE_PRUNE_PERIOD));
        cache_timer.async_wait(
          boost::bind(&service_discovery::prune_cache, this, boost::asio::placeholders::error));
    }

    void notify_observer(const service_message& entry, const DiscoveryEvent& ev)
    {
        for (const auto& observer : observerMap) {
            if (!observer->apply_filter(entry)) {
                continue;
            }
            if (ev == DiscoveryEvent::BIRTH) {
                observer->onNewServiceDiscovered(entry);
            } else if (ev == DiscoveryEvent::DEATH) {
                observer->onServiceGoodBye(entry);
            } else if (ev == DiscoveryEvent::HEARTBEAT) {
                observer->onHeartBeat(entry);
            }
        }
    }
};