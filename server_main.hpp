#ifndef TOR_PIR_SERVER_HPP
#define TOR_PIR_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <memory>
#include <chrono>
#include "seal/seal.h"
#include "pir_server.hpp"

namespace aio = boost::asio;
using boost::asio::ip::tcp;

typedef std::vector<uint8_t> PlainDatabase;  // the database to be used via tor accesses
typedef uint64_t PlainQuery;                  // the index of the element to retrieve
typedef std::vector<uint8_t> PlainReply;     // the bytes of the database entry at the index specified by PlainQuery

enum Setup{Plain, PIR, NUMBER_OF_TYPES};
struct ServerConfig {
    unsigned short port;  // port to run the server on
    std::uint8_t setup;        // whether the server is a {plain, PIR}-server
    std::uint64_t ele_num;     // number of elements in the database
    std::uint64_t ele_size;    // size (in bytes) of a given element in the database
};
extern ServerConfig server_config;


class PlainServer {
public:
    explicit PlainServer(ServerConfig &server_configs);

    PlainReply generate_reply(PlainQuery query);

    // NOTE: server takes over ownership of db and frees it when it exits.
    // Caller cannot free db
    void set_database(std::unique_ptr<std::vector<uint8_t>> &&db);

private:
    std::unique_ptr<PlainDatabase> db_;
    ServerConfig server_config_;
};


#define DEFAULT_SETUP (Plain)
#define DEFAULT_PORT (8080)
#define DEFAULT_ELE_NUM (2048)
#define DEFAULT_ELE_SIZE (128)  // Measured in bytes

#endif //TOR_PIR_SERVER_HPP
