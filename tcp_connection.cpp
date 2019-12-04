#include "tcp_connection.hpp"


tcp_connection::tcp_connection(aio::io_context& io_context, PlainServer plain_server) :
    socket_(io_context),
    plain_server_(move(plain_server)) {}

tcp_connection::tcp_connection(aio::io_context& io_context, PIRServer pir_server) :
        socket_(io_context),
        pir_server_(move(pir_server)) {}

tcp_connection::pointer tcp_connection::create(aio::io_context& io_context, PlainServer plain_server) {
    return pointer(new tcp_connection(io_context, move(plain_server)));
}

tcp_connection::pointer tcp_connection::create(aio::io_context& io_context, PIRServer pir_server) {
    return pointer(new tcp_connection(io_context, move(pir_server)));
}

tcp::socket& tcp_connection::socket() {
    return tcp_connection::socket_;
}

void tcp_connection::handle_plain_read(const boost::system::error_code&, size_t) {

    // TODO: error check and guard the user input
    std::cout << "Client wants element at index " << message_ << std::endl;

    auto time_before = std::chrono::high_resolution_clock::now();
    PlainReply reply = plain_server_->generate_reply(std::stoull(message_));
    auto time_after = std::chrono::high_resolution_clock::now();
    auto time_difference = std::chrono::duration_cast<std::chrono::microseconds>(time_after - time_before).count();
    std::cout << "Time to generate reply: " << time_difference << " micoseconds" << std::endl;

    aio::async_write(socket_,
             aio::buffer(reply),
             boost::bind(&tcp_connection::handle_write,
                         shared_from_this(),
                         aio::placeholders::error,
                         aio::placeholders::bytes_transferred));

    message_.clear();
    start_plain();  // Repeat the process all over again
}

void tcp_connection::handle_pir_read(const boost::system::error_code&, size_t) {

    PirReply reply = pir_server_->generate_reply( /* Client query */, 0);

}

void tcp_connection::handle_write(const boost::system::error_code&, size_t) {}

void tcp_connection::start_plain() {

    aio::async_read_until(socket_, aio::dynamic_buffer(message_), "\n", boost::bind(&tcp_connection::handle_plain_read,
                                                           shared_from_this(),
                                                           aio::placeholders::error,
                                                           aio::placeholders::bytes_transferred));

    aio::async_write(socket_,
            aio::buffer("Index of element to retrieve: "),
            boost::bind(&tcp_connection::handle_write,
                    shared_from_this(),
                    aio::placeholders::error,
                    aio::placeholders::bytes_transferred));
}

void tcp_connection::start_pir() {

    aio::async_read_until(socket_, aio::dynamic_buffer(message_), "\n", boost::bind(&tcp_connection::handle_plain_read,
                                                                                    shared_from_this(),
                                                                                    aio::placeholders::error,
                                                                                    aio::placeholders::bytes_transferred));

    aio::async_write(socket_,
                     aio::buffer("Index of element to retrieve: "),
                     boost::bind(&tcp_connection::handle_write,
                                 shared_from_this(),
                                 aio::placeholders::error,
                                 aio::placeholders::bytes_transferred));
}

