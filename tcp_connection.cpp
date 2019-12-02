#include "tcp_connection.hpp"


tcp_connection::tcp_connection(aio::io_context& io_context) : socket_(io_context) {}

tcp_connection::pointer tcp_connection::create(aio::io_context& io_context) {
    return pointer(new tcp_connection(io_context));
}

tcp::socket& tcp_connection::socket() {
    return tcp_connection::socket_;
}

void tcp_connection::handle_read(const boost::system::error_code&, size_t) {

    // TODO: error check and guard the user input
    std::cout << "Client wants element at index " << message_ << std::endl;

    uint64_t index = std::stoull(message_);
    std::vector<uint8_t> access;
    for (uint64_t i = 0; i < params_->item_size; i++) {
        access.push_back((*(params_->db_ptr))[(index * params_->item_size + i)]);
    }

    aio::async_write(socket_,
             aio::buffer(access),
             boost::bind(&tcp_connection::handle_write,
                         shared_from_this(),
                         aio::placeholders::error,
                         aio::placeholders::bytes_transferred));

}

void tcp_connection::handle_write(const boost::system::error_code&, size_t) {}

void tcp_connection::start(server_params* params) {

    std::cout << "[+] New connection: " << socket_.local_endpoint()
              << " (local) <---> " << socket_.remote_endpoint()
              << " (remote)" << std::endl;

    params_ = params;

    aio::async_read_until(socket_, aio::dynamic_buffer(message_), "\n", boost::bind(&tcp_connection::handle_read,
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

