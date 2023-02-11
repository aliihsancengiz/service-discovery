#include "cache.hpp"
#include "service_discovery.hpp"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

struct ServiceDiscoveredLogger : IServiceDiscovery
{
    void onNewServiceDiscovered(const service_message& msg) override
    {
        std::cout << "DISCOVERED.Service UUID : " << msg.uuid << std::endl;
    }
    void onServiceGoodBye(const service_message& msg) override
    {
        std::cout << "GOODBYE. Service UUID : " << msg.uuid << std::endl;
    }
    void onHeartBeat(const service_message& msg) override
    {
        std::cout << "HEARTBEAT. Service UUID : " << msg.uuid << std::endl;
    }
};

int main()
{
    try {
        boost::asio::io_service io_service;
        connection_config cfg("224.0.0.251", 1223);
        service_discovery disc(io_service, cfg);

        disc.register_observer(std::make_shared<ServiceDiscoveredLogger>());

        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}