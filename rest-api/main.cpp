#include <drogon/drogon.h>
#include <iostream>
#include "Inc/service_discovery.hpp"

using namespace drogon;

struct DiscoveryInterfaceController
{

  private:

};

int main()
{
    LOG_INFO << "Server running on 127.0.0.1:8848";

    app().addListener("127.0.0.1", 8848).run();

    return 1;
}