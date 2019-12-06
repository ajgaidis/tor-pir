#ifndef TOR_PIR_PLAIN_SERVER_HPP
#define TOR_PIR_PLAIN_SERVER_HPP

#include "tcp_server.hpp"
#include "server_main.hpp"

typedef std::vector<uint8_t> PlainDatabase;  // the database to be used via tor accesses
typedef uint64_t PlainQuery;                  // the index of the element to retrieve
typedef std::vector<uint8_t> PlainReply;     // the bytes of the database entry at the index specified by PlainQuery

class PlainServer : public TCPServer {
public:
    explicit PlainServer(aio::io_context& io_context);

    PlainReply generate_reply(PlainQuery query);

    void gen_database();

private:
    PlainReply reply_; // TODO: not needed?
    std::unique_ptr<PlainDatabase> db_;
    ServerConfig server_config_;

    // NOTE: server takes over ownership of db and frees it when it exits.
    // Caller cannot free db
    void set_database(std::unique_ptr<std::vector<uint8_t>> &&db);
};


#endif //TOR_PIR_PLAIN_SERVER_HPP
