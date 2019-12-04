#ifndef TOR_PIR_TCP_SERVER_HPP
#define TOR_PIR_TCP_SERVER_HPP

#include "server_main.hpp"
#include "tcp_connection.hpp"
#include "plain_server.hpp"
#include "pir_server.hpp"


class TCP_Server {
public:
    TCP_Server(aio::io_context &io_context, PlainServer plain_server);
    TCP_Server(aio::io_context &io_context, PIRServer pir_server);

private:
    aio::io_context& io_context_;
    tcp::acceptor acceptor_;
    std::optional<PlainServer> plain_server_;
    std::optional<PIRServer> pir_server_;

    void accept_new();
    void handle_accept(
            tcp_connection::pointer new_connection,
            const boost::system::error_code& error);
};

#endif //TOR_PIR_TCP_SERVER_HPP
