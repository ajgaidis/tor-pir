#include "server_main.hpp"
#include "tcp_server.hpp"

namespace po = boost::program_options;

server_params params;
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

    std::cout << "... Setting up database" << std::endl;
    std::cout << "    [i] Database contains " << params.num_items << " elements" << std::endl;
    std::cout << "    [i] Database elements are " << params.item_size << " bytes" << std::endl;

    std::vector<uint8_t> db;
    db.resize(params.num_items * params.item_size);
    std::random_device rd;
    for (uint64_t i = 0; i < params.num_items; i++) {
        for (uint64_t j = 0; j < params.item_size; j++) {

            // Generate random junk in byte-size pieces to fill database
            uint8_t value;
            value = (uint8_t)(rd() % 256);
            db[i * params.item_size + j] = value;

        }
    }
    params.db_ptr = &db;

    std::cout << "    [i] Database populated with "
                << params.num_items * params.item_size << " total bytes" << std::endl;
    std::cout << "[+] Database created successfully" << std::endl;
    std::cout << "[+] Server is now listening on port " << params.port << std::endl;

    /*
     * Running the server
     */
    try {

        // Server object will accept incoming client connections
        aio::io_context io_context;
        tcp_server server(io_context, &params);

        // run() will allow us to do things asynchronously
        io_context.run();

    } catch (std::exception& err) {
        std::cerr << err.what() << std::endl;
    }

    return 0;
}
