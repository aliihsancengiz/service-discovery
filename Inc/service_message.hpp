#pragma once

#include "nlohman/json.hpp"
#include "option.hpp"
#include "utils.hpp"

#include <boost/asio.hpp>

struct service_message_metric
{
    std::string name;
    std::string value;
};

struct service_message
{
    service_message(const std::string& name, const std::string domain, size_t ttl_msec)
        : _name(name), _domain(domain), _ttl_msec(ttl_msec)
    {
        _uuid = uuid::generate_uuid_v4();
    }
    service_message() = default;

    std::string name() const
    {
        return _name;
    }
    std::string domain() const
    {
        return _domain;
    }
    size_t ttl_msec() const
    {
        return _ttl_msec;
    }
    std::string uuid() const
    {
        return _uuid;
    }

    void set_name(const std::string& name)
    {
        _name = name;
    }
    void set_domain(const std::string& domain)
    {
        _domain = domain;
    }
    void set_ttl_msec(const size_t& ttl_msec)
    {
        _ttl_msec = ttl_msec;
    }
    void set_uuid(const std::string& uuid)
    {
        _uuid = uuid;
    }

    const auto& last_accessed() const
    {
        return _last_accessed;
    }
    void update_last_accessed()
    {
        _last_accessed = std::chrono::high_resolution_clock::now();
    }

    void add_metric(const service_message_metric& m)
    {
        _metrics.emplace_back(m);
    }
    auto& metrics() const
    {
        return _metrics;
    }
    void clear_metrics()
    {
        _metrics.clear();
    }

  private:
    std::vector<service_message_metric> _metrics;
    std::string _name;
    std::string _uuid;
    std::string _domain;
    size_t _ttl_msec;
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
        m.set_name(json_obj.at("name").get<decltype(m.name())>());
        m.set_uuid(json_obj.at("uuid").get<decltype(m.uuid())>());
        m.set_domain(json_obj.at("domain").get<decltype(m.domain())>());
        m.set_ttl_msec(json_obj.at("ttl_msec").get<decltype(m.ttl_msec())>());

        if (json_obj.count("metrics")) {
            nlohmann::json metric_array = json_obj.at("metrics");
            for (int i = 0; i < metric_array.size(); i++) {
                service_message_metric metric;
                metric.name = metric_array[i].at("name").get<decltype(metric.name)>();
                metric.value = metric_array[i].at("value").get<decltype(metric.value)>();
                m.add_metric(metric);
            }
        }
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
    json_obj["name"] = obj.name();
    json_obj["uuid"] = obj.uuid();
    json_obj["domain"] = obj.domain();
    json_obj["ttl_msec"] = obj.ttl_msec();

    if (obj.metrics().size()) {

        nlohmann::json metric_array = nlohmann::json::array();

        for (const auto& e : obj.metrics()) {
            nlohmann::json elem;

            elem["name"] = e.name;
            elem["value"] = e.value;
            metric_array.push_back(elem);
        }

        json_obj["metrics"] = metric_array;
    }

    return json_obj.dump();
}

}  // namespace message_serdes
