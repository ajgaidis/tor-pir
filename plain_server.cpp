//#include "plain_server.hpp"
//
//
//PlainServer::PlainServer(ServerConfig &server_config) : server_config_(server_config) {}
//
//PlainReply PlainServer::generate_reply(PlainQuery query) {
//    PlainReply reply;
//    reply.resize(server_config_.ele_size);
//    for (uint64_t i = 0; i < server_config_.ele_size; i++) {
//        reply.push_back((*db_)[(((uint64_t)index) * server_config_.ele_size + i)]);
//    }
//    return reply;
//}
//
//// Server takes over ownership of db and will free it when it exits
//void PlainServer::set_database(std::unique_ptr<std::vector<uint8_t>> &&db) {
//    if (!db) {
//        throw std::invalid_argument("db cannot be null");
//    }
//
//    db_ = move(db);
//}
//
