#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace periodic_timer {

struct PeriodicTimer
{
    typedef std::function<void()> timer_handler;
    PeriodicTimer(boost::asio::io_service& io, std::size_t period, timer_handler handler = nullptr)
        : io_service(io), timer_handle(io, boost::asio::chrono::milliseconds(period)),
          _period(period), _handler(handler)
    {
        timer_handle.async_wait(
          boost::bind(&PeriodicTimer::periodic_handler, this, boost::asio::placeholders::error));
    }

    const auto& period() const
    {
        return _period;
    }

    void set_handler(const timer_handler& handler)
    {
        _handler = handler;
    }

  private:
    boost::asio::io_service& io_service;
    boost::asio::steady_timer timer_handle;
    size_t _period;
    timer_handler _handler;
    void periodic_handler(const boost::system::error_code& /*e*/)
    {
        if (_handler) {
            _handler();
        }

        timer_handle.expires_at(timer_handle.expiry() + boost::asio::chrono::milliseconds(_period));
        timer_handle.async_wait(
          boost::bind(&PeriodicTimer::periodic_handler, this, boost::asio::placeholders::error));
    }
};
}  // namespace periodic_timer