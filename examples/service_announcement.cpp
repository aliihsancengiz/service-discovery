#include "service_announcement.hpp"

#include "utils.hpp"

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

int main()
{

    try {
        std::signal(SIGINT, signal_handler);

        service_message::ServiceMessage msg("Sample Service Announcement", "admin.udp.local", 2000);
        msg.add_metric(service_message::ServiceMessageMetric{"metric1", "value1"});
        msg.add_metric(service_message::ServiceMessageMetric{"metric2", "value2"});
        msg.add_metric(service_message::ServiceMessageMetric{"metric3", "value3"});

        service_message::ServiceMessage msg2("Sample Service Announcement 2", "_ali.local.udp",
                                             1000);

        connection_config cfg("224.0.0.251", 1223);
        service_announcment::ServiceAnnouncement announcer(io_service, cfg);
        announcer.start_announce({msg, msg2});
        io_service.run();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
