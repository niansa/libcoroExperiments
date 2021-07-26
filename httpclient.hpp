#ifndef _HTTPCLIENT_HPP
#define _HTTPCLIENT_HPP
#include <coro/coro.hpp>
#include <string>
#include <vector>
#include <utility>
#include <chrono>
#include <string_view>
#include <sstream>
#include <functional>
#include <memory>



class HttpClient {
    std::shared_ptr<coro::io_scheduler> scheduler;
    coro::net::ip_address addr;
    std::string host;
    uint16_t port = 0;
    std::chrono::milliseconds timeout = std::chrono::milliseconds(0);
    std::unique_ptr<coro::net::ssl_context> ssl_context = nullptr;

public:
    using Header = std::pair<std::string, std::string>;
    using Headers = std::vector<Header>;

    HttpClient(std::weak_ptr<coro::io_scheduler> sched) : scheduler(sched) {}

    bool set_ssl(bool enabled) {
        if (enabled && !ssl_context) {
            try {
                ssl_context = std::make_unique<coro::net::ssl_context>();
            } catch (...) {
                return false;
            }
        } else if (!enabled && ssl_context) {
            ssl_context.reset();
        }
        return true;
    }

    void set_addr(const std::string& addrStr) {
        addr = coro::net::ip_address::from_string(addrStr);
    }
    void set_addr(const coro::net::ip_address& addr) {
        this->addr = addr;
    }

    coro::task<bool> set_host(const std::string& host) {
        this->host = host;
        // Resolve host
        coro::net::dns_resolver resolver(scheduler, timeout);
        auto ips = (co_await resolver.host_by_name(coro::net::hostname(host)))->ip_addresses();
        if (ips.empty()) {
            co_return false;
        } else {
            addr = ips[0];
            co_return true;
        }
    }

    void set_port(uint16_t port) {
        this->port = port;
    }

    void set_timeout(std::chrono::milliseconds timeout) {
        this->timeout = timeout;
    }
    void set_timeout(time_t timeout) {
        this->timeout = std::chrono::milliseconds(timeout);
    }

    coro::task<bool> send(std::string_view path, std::function<coro::task<bool> (std::span<const char>)> cb, const Headers& headers = {});
};
#endif
