#ifndef TOR_PIR_TCP_CONNECTION_HPP
#define TOR_PIR_TCP_CONNECTION_HPP

#include "server_main.hpp"

//class TCPConnection : public boost::enable_shared_from_this<TCPConnection> {
//public:
//    typedef boost::shared_ptr<TCPConnection> pointer;
//
//    static pointer create(aio::io_context& io_context);
//
//    tcp::socket& socket();
//
//    void start();
//
//private:
//    tcp::socket socket_;
//    std::string message_;
//
//    explicit TCPConnection(aio::io_context &io_context);
//
//    void handle_read_echo(const boost::system::error_code&, size_t);
//    void handle_write_echo(const boost::system::error_code&, size_t);
////    void handle_read_plain(const boost::system::error_code&, size_t);
////    void handle_write_plain(const boost::system::error_code&, size_t);
////    void handle_read_pir(const boost::system::error_code&, size_t);
////    void handle_write_pir(const boost::system::error_code&, size_t);
//};

#endif //TOR_PIR_TCP_CONNECTION_HPP
