#include "httpclient.hpp"

#include <coro/coro.hpp>
#include <iostream>
#include <memory>



int main() {
    auto scheduler = std::make_shared<coro::io_scheduler>();

    coro::sync_wait(coro::when_all([&] () -> coro::task<void> {
                                       HttpClient client(scheduler);
                                       co_await client.set_host("example.com");
                                       //client.set_ssl(true);
                                       HttpClient::Headers headers = {{"User-Agent", "Lol"}, {"Bla", "Test"}};
                                       co_await client.send("/", [] (std::span<const char> data) -> coro::task<bool> {
                                           std::cout << std::string_view{data.data(), data.size()} << std::flush;
                                           co_return true;
                                       }, headers);
                                   }()));
}
