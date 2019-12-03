#include "server_main.hpp"
#include "tcp_server.hpp"

namespace po = boost::program_options;

Params params;
PirParams pir_params;
uint is_pir;


int main(int argc, char **argv) {

    // Define the options the program can take
    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "display help menu")
            ("pir", po::value<uint>(&is_pir)->default_value(DEFAULT_SETUP), "set PIR-based server")
            ("port,p", po::value<unsigned short>(&params.port)->default_value(DEFAULT_PORT),
                    "set port number for receiving connections")
            ("num_items", po::value<uint64_t>(&params.num_items)->default_value(DEFAULT_NUM_ITEMS),
                    "set number of items in the database")
            ("item_size", po::value<uint64_t>(&params.item_size)->default_value(DEFAULT_ITEM_SIZE),
                    "set the size of the database elements")
    ;

    po::positional_options_description p;
    p.add("port", 1); // second argument specifies the number of position arguments that will take this name
    p.add("num_items", 1);
    p.add("item_size", 1);

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
        if (NUMBER_OF_TYPES <= is_pir) {
            std::cerr << "[-] Invalid setup number.\nAborting..." << std::endl;
            return 1;
        }
        // Note: the below line should be changed when more setup options are added
        std::cout << "[i] Configuring " << (is_pir ? "PIR" : "plain") << " server" << std::endl;
    }

    if (vm.count("port")) {
        if (params.port < 1 || 65535 < params.port) {
            std::cerr << "[-] Invalid port number\nAborting..." << std::endl;
            return 1;
        }
        std::cout << "[i] Port set to " << params.port << std::endl;
    }

    // TODO: error catch the size and num items in the db

    std::cout << "[i] Setting up database" << std::endl;
    std::cout << "    [i] Database contains " << params.num_items << " elements" << std::endl;
    std::cout << "    [i] Database elements are " << params.item_size << " bytes" << std::endl;

    if (is_pir) {

        uint32_t N = 2048;
        uint32_t logt = 12;
        uint32_t d = 5;
        seal::EncryptionParameters enc_params(seal::scheme_type::BFV);
        gen_params(params.num_items, params.item_size, N, logt, d, enc_params, pir_params);

        cout << "done" << endl;

//        // Create test database
//        auto db(std::make_unique<uint8_t[]>(params.num_items * params.item_size));
//
//        std::random_device rd;
//        for (uint64_t i = 0; i < params.num_items; i++) {
//            for (uint64_t j = 0; j < params.item_size; j++) {
//                auto val = rd() % 256;
//                db.get()[(i * params.item_size) + j] = val;
//            }
//        }
//
//        // Initialize PIR Server
//        std::cout << "Initializing server and client" << std::endl;
//        PIRServer pir_server(enc_params, pir_params);
//
//        // Initialize PIR client....
//        PIRClient pir_client(enc_params, pir_params);
//        seal::GaloisKeys galois_keys = pir_client.generate_galois_keys();
//
//        // Set galois key
//        cout << "Main: Setting Galois keys...";
//        pir_server.set_galois_key(0, galois_keys);
//
//        cout << "done" << endl;
//
//        pir_server.set_database(move(db), params.num_items, params.item_size);
//        pir_server.preprocess_database();
//        cout << "database pre processed " << endl;
//
//        // Choose an index of an element in the DB
//        uint64_t ele_index = rd() % params.num_items; // element in DB at random position
//        //uint64_t ele_index = 35;
//        cout << "Main: element index = " << ele_index << " from [0, " << params.num_items -1 << "]" << endl;
//        uint64_t index = pir_client.get_fv_index(ele_index, params.item_size);   // index of FV plaintext
//        uint64_t offset = pir_client.get_fv_offset(ele_index, params.item_size); // offset in FV plaintext
//        // Measure query generation
//        cout << "Main: FV index = " << index << ", FV offset = " << offset << endl;
//
//        PirQuery query = pir_client.generate_query(index);
//        cout << "Main: query generated" << endl;
//
//        PirReply reply = pir_server.generate_reply(query, 0);
//
//        seal::Plaintext result = pir_client.decode_reply(reply);
//
//        vector<uint8_t> elems(N * logt / 8);
//        coeffs_to_bytes(logt, result, elems.data(), (N * logt) / 8);
//
//        cout << "done" << endl;

    }
//    } else {  // default server configuration
//
//        std::vector<uint8_t> db;
//        db.resize(params.num_items * params.item_size);
//        std::random_device rd;
//        for (uint64_t i = 0; i < params.num_items; i++) {
//            for (uint64_t j = 0; j < params.item_size; j++) {
//
//                // Generate random junk in byte-size pieces to fill database
//                uint8_t value;
//                value = (uint8_t) (rd() % 256);
//                db[i * params.item_size + j] = value;
//
//            }
//        }
//        params.db_ptr = &db;
//
//        std::cout << "    [i] Database populated with "
//                  << params.num_items * params.item_size << " total bytes" << std::endl;
//        std::cout << "[i] Database created successfully" << std::endl;
//        std::cout << "[i] Server is now listening on port " << params.port << std::endl;
//
//    }

    /*
     * Running the server
     */
//    try {
//
//        // Server object will accept incoming client connections
//        aio::io_context io_context;
//        tcp_server server(io_context);
//
//        // run() will allow us to do things asynchronously
//        io_context.run();
//
//    } catch (std::exception& err) {
//        std::cerr << err.what() << std::endl;
//    }

    return 0;
}
