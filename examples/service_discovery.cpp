#include "service_discovery.hpp"

#include "cache.hpp"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace {
boost::asio::io_service io_service;
}

void signal_handler(int signal)
{
    if (!io_service.stopped()) {
        io_service.stop();
    }
}
std::string print_service_message(const service_message::ServiceMessage& msg)
{
    std::stringstream ss;
    ss << "Service UUID : " << msg.uuid() << std::endl;
    for (auto m : msg.metrics()) {
        ss << "\t" << m.name << ":" << m.value << std::endl;
    }
    return ss.str();
}

struct ServiceDiscoveredLogger : service_discovery::IServiceDiscovery
{
    ServiceDiscoveredLogger(const service_filter& filter)
        : service_discovery::IServiceDiscovery(filter)
    {
    }
    void onNewServiceDiscovered(const service_message::ServiceMessage& msg) override
    {
        std::cout << "DISCOVERED. " << print_service_message(msg);
    }
    void onServiceGoodBye(const service_message::ServiceMessage& msg) override
    {
        std::cout << "GOODBYE. " << print_service_message(msg);
    }
    void onHeartBeat(const service_message::ServiceMessage& msg) override
    {
        std::cout << "HEARTBEAT. " << print_service_message(msg);
    }
};

int main()
{
    try {
        std::signal(SIGINT, signal_handler);

        connection_config cfg("224.0.0.251", 1223);
        service_discovery::ServiceDiscovery disc(io_service, cfg);

        disc.register_observer(
          std::make_shared<ServiceDiscoveredLogger>([](const service_message::ServiceMessage& msg) {
              return msg.domain() == "admin.udp.local";
          }));

        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}