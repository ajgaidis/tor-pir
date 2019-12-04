#include "tcp_server.hpp"

TCP_Server::TCP_Server(aio::io_context& io_context, PlainServer plain_server)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), server_config.port)),
          plain_server_(move(plain_server)) {

    // Begin listening for incoming connections
    accept_new();
}

TCP_Server::TCP_Server(aio::io_context& io_context, PIRServer pir_server)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), server_config.port)),
          pir_server_(move(pir_server)) {

    // Begin listening for incoming connections
    accept_new();
}

void TCP_Server::accept_new() {
    // Create a pointer to a new tcp_connection object
    tcp_connection::pointer conn = tcp_connection::create(io_context_, move(*plain_server_));

    acceptor_.async_accept(conn->socket(),
            boost::bind(&TCP_Server::handle_accept,
                    this, conn, aio::placeholders::error));
}

void TCP_Server::handle_accept(tcp_connection::pointer conn,
                   const boost::system::error_code& error) {

    std::cout << "[+] New connection: " << conn->socket().local_endpoint()
              << " (local) <---> " << conn->socket().remote_endpoint()
              << " (remote)" << std::endl;

    if (!error) {
        conn->start();
    }

    TCP_Server::accept_new();
}