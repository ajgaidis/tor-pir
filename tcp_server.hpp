#ifndef TOR_PIR_TCP_SERVER_HPP
#define TOR_PIR_TCP_SERVER_HPP

#include "server_main.hpp"
#include <variant>

class PlainServer;
class PIRServer;

/**
 * A TCPServer represents a server that communicates over a TCP connection.
 */
class TCPServer {
public:

    /**
     * Constructor for TCPServer creates a TCPServer object
     */
    TCPServer(aio::io_context &io_context, std::variant<PlainServer*, PIRServer*>);

    // We store a pointer to one of this' derived classes
    // so that we can get the correct handlers to work
    std::variant<PlainServer*, PIRServer*> server_;

private:
    aio::io_context& io_context_;
    tcp::acceptor acceptor_;

    // The actual meat and potatoes of the Server's networking capabilities
    class TCPConnection : public boost::enable_shared_from_this<TCPConnection> {
    public:
        tcp::socket socket_;
        std::string message_;
        std::variant<PlainServer*, PIRServer*> server_;

        // A pointer to this TCPConnection
        typedef boost::shared_ptr<TCPConnection> pointer;

        // Create a new TCPConnection
        static pointer create(aio::io_context& io_context, std::variant<PlainServer*, PIRServer*> server);

        // Get the socket related to this instance
        tcp::socket& socket();

        // The `main` function of a connection, if you will
        void start();

    private:
        // Constructor for TCPConnection
        explicit TCPConnection(aio::io_context &io_context, std::variant<PlainServer*, PIRServer*>);

        // All of read/write handlers that facilitate communicating with the client
        void handle_read_echo(const boost::system::error_code&, size_t);
        void handle_write_echo(const boost::system::error_code&, size_t);
        void handle_read_plain(const boost::system::error_code&, size_t);
        void handle_write_plain(const boost::system::error_code&, size_t);
        void handle_read_galkeys(const boost::system::error_code&, size_t);
        void handle_read_pir(const boost::system::error_code&, size_t);
        void handle_write_pir(const boost::system::error_code&, size_t);
    };

    // Accept new connections from the outside world
    void accept_new();

    // Handle an accepted connection
    void handle_accept(
            TCPServer::TCPConnection::pointer& new_connection,
            const boost::system::error_code& error);
};

#endif //TOR_PIR_TCP_SERVER_HPP
