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

    static bool galkey_set = false;

    if (server_config.setup == Plain) {
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_), "\n",
                              boost::bind(&TCPConnection::handle_read_plain,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));
    } else if (!galkey_set) {
//    } else if (server_config.setup == PIR) {
        // We start with a galois key exchange handshake. Nothing fancy or secure, just direct
        // transmission to get the server synced to the client. After the `handle_read_galkeys`
        // handler is exited control is switched to the main alternation between
        // `handle_write_pir` and `handle_read_pir`.
        galkey_set = true;
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_), delimiter,
                              boost::bind(&TCPConnection::handle_read_galkeys,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));
    } else {
        aio::async_read_until(socket_,
                              aio::dynamic_buffer(message_), delimiter,
                              boost::bind(&TCPConnection::handle_read_pir,
                                          shared_from_this(),
                                          aio::placeholders::error,
                                          aio::placeholders::bytes_transferred));
    }
}

void TCPServer::TCPConnection::handle_read_echo(const boost::system::error_code& err, size_t) {

    // TODO: error check and guard the user input

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

void TCPServer::TCPConnection::handle_read_plain(const boost::system::error_code& err, size_t) {

    // TODO: error check and guard the user input

    // Cast the PlainServer now that we know its what we want
    // TODO: double check this pointer madness
    auto pln_ptr = std::get_if<PlainServer*>(&server_);

    auto time_before = std::chrono::high_resolution_clock::now();
    uint64_t query;
    try { // TODO: still getting error here
        query  = std::stoull(message_);
    } catch (std::exception& err) {
        std::cerr << "Yikes! Bad conversion from string to uint64_t." << std::endl;
    }
    PlainReply reply = (*pln_ptr)->generate_reply(query);
    auto time_after = std::chrono::high_resolution_clock::now();
    auto time_difference = std::chrono::duration_cast<std::chrono::microseconds>(time_after - time_before).count();
    std::cout << "Time to generate reply: " << time_difference << " micoseconds" << std::endl;

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

void TCPServer::TCPConnection::handle_read_galkeys(const boost::system::error_code& err, size_t bytes_transferred) {

    std::cout << "Inside Galkeys. Bytes transferred: " << bytes_transferred << std::endl;

    std::cout << "Reading client's galois keys" << std::endl;
    auto pir_ptr = std::get_if<PIRServer*>(&server_);
    std::cout << "Erasing delimiter" << std::endl;
    message_.erase(message_.length() - delimiter.size(), delimiter.size());
    std::cout << "Deserializing galkeys" << std::endl;
    seal::GaloisKeys* galkeys = deserialize_galoiskeys(message_);
    std::cout << "Setting galkeys" << std::endl;
    (*pir_ptr)->set_galois_key(0, *galkeys);

    std::cout << "Done." << std::endl;

    message_.clear();
    start();


//    if (!err) {
//        aio::async_write(socket_,
//                         aio::buffer("[SERVER] Galois keys received successfully"),
//                         boost::bind(&TCPConnection::handle_write_pir,
//                                     shared_from_this(),
//                                     aio::placeholders::error,
//                                     aio::placeholders::bytes_transferred));
//
//        message_.clear();
//
//        aio::async_read_until(socket_,
//                              aio::dynamic_buffer(message_), delimiter,
//                              boost::bind(&TCPConnection::handle_write_pir,
//                                          shared_from_this(),
//                                          aio::placeholders::error,
//                                          aio::placeholders::bytes_transferred));
//        message_.clear();
//    } else {
//        delete this;
//    }
}

void TCPServer::TCPConnection::handle_read_pir(const boost::system::error_code& err, size_t bytes_transferred) {

    if (bytes_transferred == 0) {
        exit(0);
    }
    std::cout << bytes_transferred << std::endl;

    std::cout << "-> Handling PIR read" << std::endl;
    auto pir_ptr = std::get_if<PIRServer*>(&server_);
    // Database dimensions are 1 and for now we only read 1 result
    std::cout << "-> Cast server" << std::endl;
    message_.erase(message_.length() - delimiter.size(), delimiter.size());
    std::cout << "-> Cleaned message" << std::endl;
    std::cout << "-> Cleaned message size: " << message_.length() << std::endl;

    PirQuery query = deserialize_query(1, 1, message_, CIPHER_SIZE); // TODO: check this! don't know if length of ctext == element size
    std::cout << "-> Deserialized query" << std::endl;
    PirReply reply = (*pir_ptr)->generate_reply(query, 0); // 0 is the client id (so we know how to index the galois key map)
    std::cout << "-> Generated reply" << std::endl;
    std::string serialized_reply = serialize_ciphertexts(reply);
    std::cout << "-> Serialized reply" << std::endl;
    std::cout << "-> Serialized reply length: " << serialized_reply.length() << std::endl;

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