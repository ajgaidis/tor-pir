#include "plain_server.hpp"
#include "pir_server.hpp"
#include "tcp_server.hpp"


PlainServer::PlainServer(aio::io_context& io_context) :
    server_config_(server_config),
    TCPServer(io_context, std::variant<PlainServer*, PIRServer*>(this)) {}

PlainReply PlainServer::generate_reply(PlainQuery query) {
    PlainReply reply;
    reply.resize(server_config_.ele_size);
    for (uint64_t i = 0; i < server_config_.ele_size; i++) {
        reply.push_back((*db_)[(query * server_config_.ele_size + i)]);
    }
    return reply;
}

void PlainServer::gen_database() {
    PlainDatabase db;
    db.resize(server_config.ele_num * server_config.ele_size);
    std::random_device rd;

    for (uint64_t i = 0; i < server_config.ele_num; i++) {
        for (uint64_t j = 0; j < server_config.ele_size; j++) {

            // Generate random junk in byte-size pieces to fill database
            uint8_t value;
            value = (uint8_t) (rd() % 256);
            db[i * server_config.ele_size + j] = value;
        }
    }

    // Persist the database
    set_database(std::make_unique<std::vector<uint8_t>>(std::move(db)));
}

// Server takes over ownership of db and will free it when it exits
void PlainServer::set_database(std::unique_ptr<std::vector<uint8_t>> &&db) {
    if (!db) {
        throw std::invalid_argument("db cannot be null");
    }

    db_ = move(db);
}