#pragma once
#include "option.hpp"

#include <chrono>
#include <iostream>
#include <map>
#include <vector>

template<typename value>
struct expiry_policy
{
    explicit expiry_policy() = default;
    virtual ~expiry_policy() = default;
    virtual bool is_valid(const value& val)
    {
        return false;
    };
};

template<typename value>
struct time_based_expiry_policy : expiry_policy<value>
{
    explicit time_based_expiry_policy() = default;
    virtual ~time_based_expiry_policy() override = default;

    virtual bool is_valid(const value& val) override;
};

struct object_access
{
    explicit object_access() = default;
    virtual ~object_access() = default;
    virtual void update() {}
};

struct last_access : object_access
{
    explicit last_access()
    {
        update();
    }
    virtual ~last_access() override = default;

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
    explicit Cache() = default;
    ~Cache() = default;

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

    Option<value_type> at(key_type key) const
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
            if (!_policy.is_valid(it->second)) {
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
