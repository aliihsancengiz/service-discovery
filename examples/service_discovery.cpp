#include "service_discovery.hpp"

#include "cache.hpp"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

std::string print_service_message(const service_message& msg)
{
    std::stringstream ss;
    ss << "Service UUID : " << msg.uuid().substr(0, 5) << std::endl;
    for (auto m : msg.metrics()) {
        ss << "\t" << m.name << ":" << m.value << std::endl;
    }
    return ss.str();
}
struct ServiceDiscoveredLogger : service_discovery::IServiceDiscovery
{
    ServiceDiscoveredLogger(const service_filter& filter) : service_discovery::IServiceDiscovery(filter) {}
    void onNewServiceDiscovered(const service_message& msg) override
    {
        std::cout << "DISCOVERED. " << print_service_message(msg);
    }
    void onServiceGoodBye(const service_message& msg) override
    {
        std::cout << "GOODBYE. " << print_service_message(msg);
    }
    void onHeartBeat(const service_message& msg) override
    {
        std::cout << "HEARTBEAT. " << print_service_message(msg);
    }
};

int main()
{
    try {
        boost::asio::io_service io_service;
        connection_config cfg("224.0.0.251", 1223);
        service_discovery::ServiceDiscovery disc(io_service, cfg);

        disc.register_observer(
          std::make_shared<ServiceDiscoveredLogger>([](const service_message& msg) {
              //   return msg.domain == "admin.udp.local";
              return true;
          }));

        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}