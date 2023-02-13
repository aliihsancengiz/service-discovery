#pragma once
#include "option.hpp"

#include <chrono>
#include <iostream>
#include <map>
#include <vector>

template<typename value>
struct expiry_policy
{
    virtual bool is_expired(const value& val)
    {
        return false;
    };
};

template<typename value>
struct time_based_expiry_policy : expiry_policy<value>
{
    virtual bool is_expired(const value& val) override;
};

struct object_access
{
    virtual void update() {}
};

struct last_access : object_access
{
    last_access()
    {
        update();
    }
    const auto& last_accessed() const
    {
        return _last_accessed;
    }
    virtual void update() override final
    {
        _last_accessed = std::chrono::high_resolution_clock::now();
    }

  private:
    std::chrono::system_clock::time_point _last_accessed;
};

template<typename key_type, typename value_type = object_access,
         typename policy = expiry_policy<value_type>>
struct Cache
{
    void add(key_type key, value_type value)
    {
        value.update();
        cache_data_holder.emplace(key, value);
    }

    void remove(key_type key)
    {
        cache_data_holder.erase(key);
    }

    bool has(key_type key)
    {
        return cache_data_holder.find(key) != cache_data_holder.end();
    }

    Option<value_type> at(key_type key)
    {
        Option<value_type> res;
        try {
            auto val = cache_data_holder.at(key);
            res.set_some(val);
        } catch (const std::exception& e) {
            res.reset();
        }
        return res;
    }

    void update(key_type key, value_type value)
    {
        if (has(key)) {
            value.update();
            cache_data_holder[key] = value;
        }
    }

    std::vector<value_type> prune_expired_entries()
    {
        std::vector<value_type> removed_values;
        for (auto it = cache_data_holder.begin(); it != cache_data_holder.end();) {
            if (_policy.is_expired(it->second)) {
                removed_values.push_back(it->second);
                cache_data_holder.erase(it++);
            } else {
                ++it;
            }
        }
        return removed_values;
    }

  private:
    std::map<key_type, value_type> cache_data_holder;
    policy _policy;
};
