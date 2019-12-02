#include "tcp_server.hpp"

tcp_server::tcp_server(aio::io_context& io_context, server_params* params)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), params->port)),
          params_(params) {

        // Begin listening for incoming connections
        accept_new();
}

void tcp_server::accept_new() {
    // Create a pointer to a new tcp_connection object
    tcp_connection::pointer conn = tcp_connection::create(io_context_);

    acceptor_.async_accept(conn->socket(),
            boost::bind(&tcp_server::handle_accept,
                    this, conn, aio::placeholders::error));
}

void tcp_server::handle_accept(tcp_connection::pointer conn,
                   const boost::system::error_code& error) {

    if (!error) {
        conn->start(params_);
    }

    tcp_server::accept_new();
}