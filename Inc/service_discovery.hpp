#pragma once
#include "cache.hpp"
#include "connection_config.hpp"
#include "defines.hpp"
#include "periodic_timer.hpp"
#include "service_message.hpp"

#include <boost/array.hpp>

namespace service_discovery {

namespace detail {

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

    enum class DiscoveryEvent
    {
        BIRTH,
        HEARTBEAT,
        DEATH
    };

}  // namespace detail

struct IServiceDiscovery
{
    typedef std::function<bool(const service_message::ServiceMessage&)> service_filter;
    IServiceDiscovery(const service_filter& filter) : _filter(filter) {}
    IServiceDiscovery()
        : _filter([](const service_message::ServiceMessage&) {
              return true;
          })
    {
    }
    virtual void onNewServiceDiscovered(const service_message::ServiceMessage& new_service){};
    virtual void onServiceGoodBye(const service_message::ServiceMessage& msg){};
    virtual void onHeartBeat(const service_message::ServiceMessage& msg){};

    bool apply_filter(const service_message::ServiceMessage& msg)
    {
        return _filter(msg);
    }

  private:
    service_filter _filter;
};

struct ServiceDiscovery : detail::IPacketObserver
{
    ServiceDiscovery(boost::asio::io_service& io, const connection_config& cfg)
        : io_service(io), _socket(io, cfg),
          _cache_prune_timer(io, SERVICE_DISCOVERY_CACHE_PRUNE_PERIOD)
    {
        _cache_prune_timer.set_handler(std::bind(&ServiceDiscovery::prune_cache, this));
        _socket.set_packet_observer(this);
    }

    ~ServiceDiscovery()
    {
        _socket.reset_packet_observer();
    }
    void register_observer(const std::shared_ptr<IServiceDiscovery>& cb_obj)
    {
        observerMap.push_back(cb_obj);
    }

  private:
    std::vector<std::shared_ptr<IServiceDiscovery>> observerMap;
    boost::asio::io_service& io_service;
    detail::DiscoverySocket _socket;
    Cache<std::string, service_message::ServiceMessage,
          time_based_expiry_policy<service_message::ServiceMessage>>
      service_cache;
    periodic_timer::PeriodicTimer _cache_prune_timer;

    void process_packet(const std::string& recived_packet) override
    {
        auto msg_res = message_serdes::from_json<service_message::ServiceMessage>(recived_packet);
        if (msg_res.is_some()) {
            auto msg = msg_res.unwrap();
            auto cache_key = msg.uuid();

            if (service_cache.has(cache_key)) {
                service_cache.update(cache_key, msg);
                notify_observer(msg, detail::DiscoveryEvent::HEARTBEAT);
            } else {
                // New service discovered, emplace it to cache and notify observers
                service_cache.add(cache_key, msg);
                notify_observer(msg, detail::DiscoveryEvent::BIRTH);
            }
        }
    }

    void prune_cache()
    {
        for (const auto& entry : service_cache.prune_expired_entries()) {
            notify_observer(entry, detail::DiscoveryEvent::DEATH);
        }
    }

    void notify_observer(const service_message::ServiceMessage& entry,
                         const detail::DiscoveryEvent& ev)
    {
        for (const auto& observer : observerMap) {
            if (!observer->apply_filter(entry)) {
                continue;
            }
            if (ev == detail::DiscoveryEvent::BIRTH) {
                observer->onNewServiceDiscovered(entry);
            } else if (ev == detail::DiscoveryEvent::DEATH) {
                observer->onServiceGoodBye(entry);
            } else if (ev == detail::DiscoveryEvent::HEARTBEAT) {
                observer->onHeartBeat(entry);
            }
        }
    }
};
}  // namespace service_discovery
