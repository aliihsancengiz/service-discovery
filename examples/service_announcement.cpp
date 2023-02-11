#include "service_announcement.hpp"

#include <iostream>
#include "utils.hpp"

int main()
{
    try {
        boost::asio::io_service io_service;
        service_message msg;
        msg.name = "Sample Service Announcement";
        msg.uuid = uuid::generate_uuid_v4();
        msg.role = "admin";
        msg.ttl = 2000;
        msg.update_last_accessed();

        service_message msg2;
        msg2.name = "Sample Service Announcement 2";
        msg2.uuid = uuid::generate_uuid_v4();
        msg2.role = "admin";
        msg2.ttl = 1000;
        msg2.update_last_accessed();

        connection_config cfg("224.0.0.251", 1223);
        service_announcement announcer(io_service, cfg);
        announcer.start_announce({msg, msg2});
        io_service.run();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
