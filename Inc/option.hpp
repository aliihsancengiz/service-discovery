#pragma once

#include <optional>

template<typename Value>
struct Option
{
    Option() = default;

    Option(Value v)
    {
        mValue = v;
    }

    void set_some(Value v)
    {
        mValue = v;
    }

    constexpr bool is_some() const
    {
        return mValue.has_value();
    }
    constexpr bool is_none() const
    {
        return !mValue.has_value();
    }
    constexpr explicit operator bool() const
    {
        return mValue.has_value();
    }

    constexpr Value unwrap() const
    {
        if (is_some()) {
            return mValue.value();
        }
        throw std::exception();
    }

    constexpr Value unwrap_or(Value v) const
    {
        if (is_some()) {
            return mValue.value();
        }
        return v;
    }

    constexpr void reset()
    {
        mValue = std::nullopt;
    }

  private:
    std::optional<Value> mValue;
};