#ifndef TOR_PIR_TCP_SERVER_HPP
#define TOR_PIR_TCP_SERVER_HPP

#include "server_main.hpp"
#include "tcp_connection.hpp"

class tcp_server {
public:
    explicit tcp_server(aio::io_context &io_context, server_params* params);

private:
    aio::io_context& io_context_;
    tcp::acceptor acceptor_;
    server_params* params_;

    void accept_new();
    void handle_accept(
            tcp_connection::pointer new_connection,
            const boost::system::error_code& error);
};


#endif //TOR_PIR_TCP_SERVER_HPP
