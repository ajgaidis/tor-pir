#ifndef TOR_PIR_TCP_SERVER_HPP
#define TOR_PIR_TCP_SERVER_HPP

#include "server_main.hpp"
#include <variant>

class PlainServer;
class PIRServer;

class TCPServer {
public:
    TCPServer(aio::io_context &io_context, std::variant<PlainServer*, PIRServer*>);

    std::variant<PlainServer*, PIRServer*> server_;

private:
    aio::io_context& io_context_;
    tcp::acceptor acceptor_;

    class TCPConnection : public boost::enable_shared_from_this<TCPConnection> {
    public:
        tcp::socket socket_;
        std::string message_;
        std::variant<PlainServer*, PIRServer*> server_;

        typedef boost::shared_ptr<TCPConnection> pointer;

        static pointer create(aio::io_context& io_context, std::variant<PlainServer*, PIRServer*> server);

        tcp::socket& socket();

        void start();

    private:
        explicit TCPConnection(aio::io_context &io_context, std::variant<PlainServer*, PIRServer*>);

        void handle_read_echo(const boost::system::error_code&, size_t);
        void handle_write_echo(const boost::system::error_code&, size_t);
        void handle_read_plain(const boost::system::error_code&, size_t);
        void handle_write_plain(const boost::system::error_code&, size_t);
        void handle_read_galkeys(const boost::system::error_code&, size_t);
        void handle_read_pir(const boost::system::error_code&, size_t);
        void handle_write_pir(const boost::system::error_code&, size_t);
    };

    void accept_new();
    void handle_accept(
            TCPServer::TCPConnection::pointer& new_connection,
            const boost::system::error_code& error);
};

#endif //TOR_PIR_TCP_SERVER_HPP
