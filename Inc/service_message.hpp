#pragma once

#include "nlohman/json.hpp"
#include "option.hpp"

#include <boost/asio.hpp>

struct service_message
{
    std::string name;
    std::string uuid;
    std::string domain;
    size_t ttl;

    const auto& last_accessed() const
    {
        return _last_accessed;
    }
    void update_last_accessed()
    {
        _last_accessed = std::chrono::high_resolution_clock::now();
    }

  private:
    std::chrono::system_clock::time_point _last_accessed;
};

namespace message_serdes {

template<typename Obj>
Option<Obj> from_json(const std::string& json_str);

template<typename Obj>
std::string to_json(const Obj& obj);

template<>
Option<service_message> from_json<service_message>(const std::string& json_str)
{
    Option<service_message> ret;
    service_message m;
    try {

        nlohmann::json json_obj = nlohmann::json::parse(json_str);
        m.name = json_obj.at("name").get<decltype(m.name)>();
        m.uuid = json_obj.at("uuid").get<decltype(m.uuid)>();
        m.domain = json_obj.at("domain").get<decltype(m.domain)>();
        m.ttl = json_obj.at("ttl").get<decltype(m.ttl)>();
    } catch (const std::exception& e) {
        return ret;
    }
    ret.set_some(m);
    return ret;
}

template<>
std::string to_json<service_message>(const service_message& obj)
{
    nlohmann::json json_obj;
    json_obj["name"] = obj.name;
    json_obj["uuid"] = obj.uuid;
    json_obj["domain"] = obj.domain;
    json_obj["ttl"] = obj.ttl;

    return json_obj.dump();
}

}  // namespace message_serdes
