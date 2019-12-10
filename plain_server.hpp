#ifndef TOR_PIR_PLAIN_SERVER_HPP
#define TOR_PIR_PLAIN_SERVER_HPP

#include "tcp_server.hpp"
#include "server_main.hpp"

typedef std::vector<uint8_t> PlainDatabase;   // the database to be used via tor accesses
typedef uint64_t PlainQuery;                  // the index of the element to retrieve
typedef std::vector<uint8_t> PlainReply;      // the bytes of the database entry at the index specified by PlainQuery

/**
 * A PlainServer object is a server that allows for the querying
 * of a database represented as a 1-D array of uint8_t's.
 */
class PlainServer : public TCPServer {
public:

    /**
     * Constructor creates an instance of a PlainServer as well as
     * an instance of the TCPServer base class
     */
    explicit PlainServer(aio::io_context& io_context);

    /**
     * Generates a reply to a database query; that is, the element
     * at the given query index
     */
    PlainReply generate_reply(PlainQuery query);

    /**
     * Generates a database that adheres to the database specifications
     * passed into `main()` from the user
     */
    void gen_database();

private:
    PlainReply reply_;
    std::unique_ptr<PlainDatabase> db_;
    ServerConfig server_config_;

    // NOTE: server takes over ownership of db and frees it when it exits.
    // Caller cannot free db
    void set_database(std::unique_ptr<std::vector<uint8_t>> &&db);
};


#endif //TOR_PIR_PLAIN_SERVER_HPP
