#ifndef TOR_PIR_TCP_CONNECTION_HPP
#define TOR_PIR_TCP_CONNECTION_HPP

#include "server_main.hpp"

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(aio::io_context& io_context);

    tcp::socket& socket();

    void start(server_params* params);

private:
    tcp::socket socket_;
    std::string message_;
    server_params* params_;

    explicit tcp_connection(aio::io_context& io_context);

    void handle_read(const boost::system::error_code&, size_t);

    void handle_write(const boost::system::error_code&, size_t);
};


#endif //TOR_PIR_TCP_CONNECTION_HPP
