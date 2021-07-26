#include "httpclient.hpp"

#include <cerrno>



coro::task<bool> HttpClient::send(std::string_view path, std::function<coro::task<bool> (std::span<const char>)> cb, const Headers& headers) {
    co_await scheduler->schedule();

    // Connect
    coro::net::tcp_client client{scheduler, {
            .address = {addr},
                    .port = port?port:uint16_t(ssl_context?443:80),
                    .ssl_ctx = ssl_context.get()
        }
                                };
    co_await client.connect(timeout);
    if (!client.socket().is_valid()) {
        co_return false;
    }

    if (ssl_context) {
        // SSL handshake
        if (co_await client.ssl_handshake() != coro::net::ssl_handshake_status::ok) {
            co_return false;
        }
    }

    // Build request
    std::string reqStr;
    {
        std::ostringstream req;
        req << "GET " << path << " HTTP/1.1\n"
               "Host: " << (host.empty()?addr.to_string():host) << "\n";
        for (const auto& [key, value] : headers) {
            req << key << ": " << value << '\n';
        }
        req << '\n';
        reqStr = req.str();
    }

    // Send Request
    std::span<const char> remaining = reqStr;
    while (true) {
        auto [send_status, r] = client.send(remaining);
        if (send_status != coro::net::send_status::ok) {
            co_return false;
        }

        if (r.empty()) {
            break;
        }

        remaining = r;
        auto pstatus = co_await client.poll(coro::poll_op::write, timeout);
        if (pstatus != coro::poll_status::event) {
            co_return false;
        }
    }

    // Receive response
    std::vector<char> response(256);
    while (true) {
        auto pres = co_await client.poll(coro::poll_op::read, timeout);
        // Check if poll succeeded
        if (pres == coro::poll_status::event) {
            // Receive
            auto [recv_status, recv_bytes] = client.recv(response);
            // Handle error in receive
            if (recv_status != coro::net::recv_status::ok || !(co_await cb(recv_bytes))) {
                co_return recv_status == coro::net::recv_status::closed;
            }
        } else {
            co_return pres != coro::poll_status::closed;
        }
    }
}
