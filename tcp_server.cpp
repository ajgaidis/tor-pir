#include "tcp_server.hpp"
#include "plain_server.hpp"
#include "pir_server.hpp"
#include "pir.hpp"
#include <variant>

TCPServer::TCPServer(aio::io_context& io_context, std::variant<PlainServer*, PIRServer*> server)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), server_config.port)),
          server_(server) {

    // Begin listening for incoming connections
    accept_new();
}

void TCPServer::accept_new() {
    // Create a pointer to a new TCPConnection object
    TCPConnection::pointer conn = TCPConnection::create(io_context_, server_);

    acceptor_.async_accept(conn->socket(),
            boost::bind(&TCPServer::handle_accept,
                    this, conn, aio::placeholders::error));
}

void TCPServer::handle_accept(
                    TCPServer::TCPConnection::pointer& conn,
                    const boost::system::error_code& error) {

    std::cout << "[+] New connection: " << conn->socket().local_endpoint()
              << " (local) <---> " << conn->socket().remote_endpoint()
              << " (remote)" << std::endl;

    if (!error) {
        conn->start();
    }

    TCPServer::accept_new();
}


TCPServer::TCPConnection::TCPConnection(aio::io_context &io_context, std::variant<PlainServer*, PIRServer*> server) :
    socket_(io_context),
    server_(server) {}

TCPServer::TCPConnection::pointer TCPServer::TCPConnection::create(aio::io_context& io_context, std::variant<PlainServer*, PIRServer*> server) {
    return pointer(new TCPConnection(io_context, server));
}

tcp::socket& TCPServer::TCPConnection::socket() {
    return TCPConnection::socket_;
}

void TCPServer::TCPConnection::start() {
    static bool is_galkey_set = false;

    if (server_config.setup == Plain) {
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_), "\n",
                              boost::bind(&TCPConnection::handle_read_plain,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));

    } else if (server_config.setup == PIR && !is_galkey_set) {
        // We start with a galois key exchange handshake. Nothing fancy or secure, just direct
        // transmission to get the server synced to the client. After the `handle_read_galkeys`
        // handler is exited control is switched to the main alternation between
        // `handle_write_pir` and `handle_read_pir`.
        is_galkey_set = true;
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_), delimiter,
                              boost::bind(&TCPConnection::handle_read_galkeys,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));

    } else if (server_config.setup == PIR && is_galkey_set) {
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_), delimiter,
                              boost::bind(&TCPConnection::handle_read_pir,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));
    }
}

void TCPServer::TCPConnection::handle_read_echo(const boost::system::error_code& err, size_t) {

    if (!err) {
        aio::async_write(socket_,
                         aio::buffer(message_),
                         boost::bind(&TCPConnection::handle_write_echo,
                                     shared_from_this(),
                                     aio::placeholders::error,
                                     aio::placeholders::bytes_transferred));

        message_.clear();
    } else {
        delete this;
    }
}

void TCPServer::TCPConnection::handle_write_echo(const boost::system::error_code& err, size_t) {

    if (!err) {
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_),"\n",
                              boost::bind(&TCPConnection::handle_read_echo,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));
    } else {
        delete this;
    }

}

void TCPServer::TCPConnection::handle_read_plain(const boost::system::error_code& err, size_t bytes_transferred) {

    if (bytes_transferred == 0) {
        exit(0);
    }

    auto pln_ptr = std::get_if<PlainServer*>(&server_);

    // auto time_before = std::chrono::high_resolution_clock::now();
    PlainReply reply = (*pln_ptr)->generate_reply(std::stoull(message_));
    // auto time_after = std::chrono::high_resolution_clock::now();
    // auto time_difference = std::chrono::duration_cast<std::chrono::microseconds>(time_after - time_before).count();
    // std::cout << "Time to generate reply: " << time_difference << " micoseconds" << std::endl;

    if (!err) {
        aio::async_write(socket_,
                         aio::buffer(reply),
                         boost::bind(&TCPConnection::handle_write_plain,
                                     shared_from_this(),
                                     aio::placeholders::error,
                                     aio::placeholders::bytes_transferred));

        message_.clear();
    } else {
        delete this;
    }
}

void TCPServer::TCPConnection::handle_write_plain(const boost::system::error_code& err, size_t) {

    if (!err) {
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_),"\n",
                              boost::bind(&TCPConnection::handle_read_plain,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));
    } else {
        delete this;
    }

}

void TCPServer::TCPConnection::handle_read_galkeys(const boost::system::error_code& err, size_t) {

    std::cout << "Reading in client's galois keys" << std::endl;
    auto pir_ptr = std::get_if<PIRServer*>(&server_);

    // Erase the delimiter that lets us know the end of the transmission
    message_.erase(message_.length() - delimiter.size(), delimiter.size());

    // Deserialize the bytes read from the network
    seal::GaloisKeys* galkeys = deserialize_galoiskeys(message_);

    // Set the server's copy of the client's galois keys. Since there will only
    // be one client for all the tests, we hardcode the 0
    (*pir_ptr)->set_galois_key(0, *galkeys);

    // We use one buffer for all the communication with the client so we need
    // to clear it so we can fill it up again
    message_.clear();

    // Now that the keys are setup we can return to `start` and begin the
    // actual PIR read/write cycle
    start();
}

void TCPServer::TCPConnection::handle_read_pir(const boost::system::error_code& err, size_t bytes_transferred) {

    if (bytes_transferred == 0) {
        exit(0);
    }

    auto pir_ptr = std::get_if<PIRServer*>(&server_);
    // Database dimensions are 1 and for now we only read 1 result
    message_.erase(message_.length() - delimiter.size(), delimiter.size());

    PirQuery query = deserialize_query(1, 1, message_, CIPHER_SIZE);
    PirReply reply = (*pir_ptr)->generate_reply(query, 0);
    std::string serialized_reply = serialize_ciphertexts(reply);
    serialized_reply.append(delimiter);

    if (!err) {
        aio::async_write(socket_,
                         aio::buffer(serialized_reply),
                         boost::bind(&TCPConnection::handle_write_pir,
                                     shared_from_this(),
                                     aio::placeholders::error,
                                     aio::placeholders::bytes_transferred));

        message_.clear();
    } else {
        delete this;
    }
}

void TCPServer::TCPConnection::handle_write_pir(const boost::system::error_code& err, size_t) {

    if (!err) {
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_), delimiter,
                              boost::bind(&TCPConnection::handle_read_pir,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));
    } else {
        delete this;
    }
}