#include "tcp_connection.hpp"


//TCPConnection::TCPConnection(aio::io_context &io_context) : socket_(io_context) {}
//
//TCPConnection::pointer TCPConnection::create(aio::io_context& io_context) {
//    return pointer(new TCPConnection(io_context));
//}
//
//tcp::socket& TCPConnection::socket() {
//    return TCPConnection::socket_;
//}
//
//void TCPConnection::start() {
//
//    aio::async_read_until(socket_,
//                          aio::dynamic_buffer(message_), "\n",
//                          boost::bind(&TCPConnection::handle_read_echo,
//                                      shared_from_this(),
//                                      aio::placeholders::error,
//                                      aio::placeholders::bytes_transferred));
//}
//
//void TCPConnection::handle_read_echo(const boost::system::error_code& err, size_t) {
//
//    // TODO: error check and guard the user input
//
//    if (!err) {
//            // async_write handles serializing the data for the socket
//            aio::async_write(socket_,
//                             aio::buffer(message_),
//                             boost::bind(&TCPConnection::handle_write_echo,
//                                         shared_from_this(),
//                                         aio::placeholders::error,
//                                         aio::placeholders::bytes_transferred));
//
//        message_.clear();
//    } else {
//        delete this;
//    }
//}
//
//void TCPConnection::handle_write_echo(const boost::system::error_code& err, size_t) {
//
//    if (!err) {
//        aio::async_read_until(socket_,
//                aio::dynamic_buffer(message_),"\n",
//                boost::bind(&TCPConnection::handle_read_echo,
//                        shared_from_this(),
//                        aio::placeholders::error,
//                        aio::placeholders::bytes_transferred));
//    } else {
//        delete this;
//    }
//
//}

//void TCPConnection::handle_read_plain(const boost::system::error_code& err, size_t) {
//
//    // TODO: error check and guard the user input
//
//    auto time_before = std::chrono::high_resolution_clock::now();
//    PlainReply reply = plain_server_->generate_reply(std::stoull(message_));
//    auto time_after = std::chrono::high_resolution_clock::now();
//    auto time_difference = std::chrono::duration_cast<std::chrono::microseconds>(time_after - time_before).count();
//    std::cout << "Time to generate reply: " << time_difference << " micoseconds" << std::endl;
//
//    if (!err) {
//        // async_write handles serializing the data for the socket
//        aio::async_write(socket_,
//                         aio::buffer(message_),
//                         boost::bind(&TCPConnection::handle_write_plain,
//                                     shared_from_this(),
//                                     aio::placeholders::error,
//                                     aio::placeholders::bytes_transferred));
//
//        message_.clear();
//    } else {
//        delete this;
//    }
//}
//
//void TCPConnection::handle_write_plain(const boost::system::error_code& err, size_t) {
//
//    if (!err) {
//        aio::async_read_until(socket_,
//                              aio::dynamic_buffer(message_),"\n",
//                              boost::bind(&TCPConnection::handle_read_plain,
//                                          shared_from_this(),
//                                          aio::placeholders::error,
//                                          aio::placeholders::bytes_transferred));
//    } else {
//        delete this;
//    }
//
//}