#include "server_main.hpp"
#include "tcp_server.hpp"
#include "pir.hpp"
#include "pir_client.hpp"

namespace po = boost::program_options;

ServerConfig server_config;
PirParams pir_params;


PlainServer::PlainServer(ServerConfig &server_configs) : server_config_(server_configs) {}

PlainReply PlainServer::generate_reply(PlainQuery query) {
    PlainReply reply;
    reply.resize(server_config_.ele_size);
    for (uint64_t i = 0; i < server_config_.ele_size; i++) {
        reply.push_back((*db_)[(query * server_config_.ele_size + i)]);
    }
    return reply;
}

// Server takes over ownership of db and will free it when it exits
void PlainServer::set_database(std::unique_ptr<std::vector<uint8_t>> &&db) {
    if (!db) {
        throw std::invalid_argument("db cannot be null");
    }

    db_ = move(db);
}


int main(int argc, char **argv) {

    // Define the options the program can take
    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "display help menu")
            ("pir", po::value<uint8_t>(&server_config.setup)->default_value(DEFAULT_SETUP), "set PIR-based server")
            ("port,p", po::value<unsigned short>(&server_config.port)->default_value(DEFAULT_PORT),
                    "set port number for receiving connections")
            ("ele_num", po::value<uint64_t>(&server_config.ele_num)->default_value(DEFAULT_ELE_NUM),
                    "set number of items in the database")
            ("ele_size", po::value<uint64_t>(&server_config.ele_size)->default_value(DEFAULT_ELE_SIZE),
                    "set the size of the database elements")
    ;

    po::positional_options_description p;
    p.add("port", 1); // second argument specifies the number of position arguments that will take this name
    p.add("ele_num", 1);
    p.add("ele_size", 1);

    // Populate a variables_map with the command line arguments (positional and flag-based)
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::store(po::command_line_parser(argc, argv).
            options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage:\n\t./server [options]\n" << desc << std::endl;
        return 1;
    }

    if (vm.count("pir")) {
        if (NUMBER_OF_TYPES <= server_config.setup) {
            std::cerr << "[-] Invalid setup number.\nAborting..." << std::endl;
            return 1;
        }
        // Note: the below lines should be changed when more setup options are added
        std::cout << "[i] Configuring "
                    << (server_config.setup ? "PIR" : "plain")
                    << " server" << std::endl;
    }

    if (vm.count("port")) {
        if (server_config.port < 1 || 65535 < server_config.port) {
            std::cerr << "[-] Invalid port number\nAborting..." << std::endl;
            return 1;
        }
        std::cout << "[i] Port set to " << server_config.port << std::endl;
    }

    // TODO: error catch the size and num items in the db

    std::cout << "[i] Setting up database" << std::endl;
    std::cout << "    [i] Database contains " << server_config.ele_num << " elements" << std::endl;
    std::cout << "    [i] Database elements are " << server_config.ele_size << " bytes" << std::endl;

    if (server_config.setup) {

        uint32_t N = 2048;
        uint32_t logt = 12;
        uint32_t d = 5;
        seal::EncryptionParameters enc_params(seal::scheme_type::BFV);
        gen_params(server_config.ele_num, server_config.ele_size, N, logt, d, enc_params, pir_params);

        // Create database
        auto db(std::make_unique<uint8_t[]>(server_config.ele_num * server_config.ele_size));
        std::random_device rd;
        for (uint64_t i = 0; i < server_config.ele_num; i++) {
            for (uint64_t j = 0; j < server_config.ele_size; j++) {
                auto val = rd() % 256;
                db.get()[(i * server_config.ele_size) + j] = val;
            }
        }

        // Initialize PIR Server
        std::cout << "Initializing server and client" << std::endl;
        PIRServer pir_server(enc_params, pir_params);

        // Initialize PIR client....
        PIRClient pir_client(enc_params, pir_params);
        seal::GaloisKeys galois_keys = pir_client.generate_galois_keys();

        // Set galois key
        cout << "Main: Setting Galois keys...";
        pir_server.set_galois_key(0, galois_keys);

        cout << "done" << endl;

        pir_server.set_database(move(db), server_config.ele_num, server_config.ele_size);
        pir_server.preprocess_database();
        cout << "database pre processed " << endl;

        // Choose an index of an element in the DB
        uint64_t ele_index = rd() % server_config.ele_num; // element in DB at random position
        //uint64_t ele_index = 35;
        cout << "Main: element index = " << ele_index << " from [0, " << server_config.ele_num -1 << "]" << endl;
        uint64_t index = pir_client.get_fv_index(ele_index, server_config.ele_size);   // index of FV plaintext
        uint64_t offset = pir_client.get_fv_offset(ele_index, server_config.ele_size); // offset in FV plaintext
        // Measure query generation
        cout << "Main: FV index = " << index << ", FV offset = " << offset << endl;

        PirQuery query = pir_client.generate_query(index);
        cout << "Main: query generated" << endl;

        PirReply reply = pir_server.generate_reply(query, 0);

        seal::Plaintext result = pir_client.decode_reply(reply);

        vector<uint8_t> elems(N * logt / 8);
        coeffs_to_bytes(logt, result, elems.data(), (N * logt) / 8);

        cout << "done" << endl;

    } else {  // default server configuration

        PlainDatabase db;
        PlainServer plain_server(server_config);
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
        plain_server.set_database(std::make_unique<std::vector<uint8_t>>(std::move(db)));

        std::cout << "    [i] Database populated with "
                  << server_config.ele_num * server_config.ele_size << " total bytes" << std::endl;
        std::cout << "[i] Database created successfully" << std::endl;
        std::cout << "[i] Server is now listening on port " << server_config.port << std::endl;

        // Server object will accept incoming client connections
        aio::io_context io_context;

        TCP_Server tcp_server(io_context, move(plain_server));

        // run() will allow us to do things asynchronously
        io_context.run();
    }

    /*
     * Running the server
     */
//    try {
//
//        // Server object will accept incoming client connections
//        aio::io_context io_context;
//        tcp_server server(io_context, plain_server);
//
//        // run() will allow us to do things asynchronously
//        io_context.run();
//
//    } catch (std::exception& err) {
//        std::cerr << err.what() << std::endl;
//    }

    return 0;
}
