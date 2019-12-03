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
#include "pir.hpp"
#include "pir_server.hpp"
#include "pir_client.hpp"
#include "seal/seal.h"

namespace aio = boost::asio;
using boost::asio::ip::tcp;

enum Setup{Plain, PIR, NUMBER_OF_TYPES};

struct Params {
    unsigned short port;
    uint64_t num_items;
    uint64_t item_size;
    std::vector<uint8_t>* db_ptr;
};
extern Params params;

#define DEFAULT_SETUP (Plain)
#define DEFAULT_PORT (8080)
#define DEFAULT_NUM_ITEMS (2048)
#define DEFAULT_ITEM_SIZE (128) // Measured in bytes

#endif //TOR_PIR_SERVER_HPP
