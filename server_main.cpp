#include "server_main.hpp"
#include "pir.hpp"
#include "plain_server.hpp"
#include "seal/seal.h"
#include "pir_server.hpp"

namespace po = boost::program_options;

ServerConfig server_config;
PirParams pir_params;
std::string delimiter = ":q!";
uint32_t N;
uint32_t logt;
uint32_t d;


int main(int argc, char **argv) {

    // Define the options the program can take
    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "display help menu")
            ("pir","set PIR-based server")
            ("port,p", po::value<unsigned short>(&server_config.port)->default_value(DEFAULT_PORT),
                    "set port number for receiving connections")
            ("ele_num", po::value<uint64_t>(&server_config.ele_num)->default_value(DEFAULT_ELE_NUM),
                    "set number of items in the database")
            ("ele_size", po::value<uint64_t>(&server_config.ele_size)->default_value(DEFAULT_ELE_SIZE),
                    "set the size of the database elements")
            ("N", po::value<uint32_t>(&N)->default_value(DEFAULT_N),
             "...")
            ("logt", po::value<uint32_t>(&logt)->default_value(DEFAULT_LOGT),
             "...")
            ("d", po::value<uint32_t>(&d)->default_value(DEFAULT_D),
             "...")
    ;

    // Allow positional arguments
    po::positional_options_description p;
    // second argument below specifies the number of position arguments that will take this name
    p.add("port", 1);
    p.add("ele_num", 1);
    p.add("ele_size", 1);
    p.add("N", 1);
    p.add("logt", 1);
    p.add("d", 1);

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

    server_config.setup = (vm.count("pir")) ? PIR : Plain;
    std::cout << "[i] Configuring "
              << (server_config.setup ? "PIR" : "plain")
              << " server" << std::endl;

    if (vm.count("port")) {
        if (server_config.port < 1 || 65535 < server_config.port) {
            std::cerr << "[-] Invalid port number\nAborting..." << std::endl;
            return 1;
        }
        std::cout << "[i] Port set to " << server_config.port << std::endl;
    }


    /*
     * Running the server
     */
    try {

        if (server_config.setup == PIR) {

            aio::io_context io_context;
            // These parameters should match what the client sets up, we don't
            // handle this in code, so it should be done out of band
            seal::EncryptionParameters enc_params(seal::scheme_type::BFV);
            gen_params(server_config.ele_num, server_config.ele_size,
                    N, logt, d, enc_params, pir_params);
            PIRServer server(io_context, enc_params, pir_params);
            server.gen_database();

            // run() will allow us to do things asynchronously
            io_context.run();


        } else {  // Plain server configuration

            aio::io_context io_context;
            PlainServer server(io_context);
            server.gen_database();

            // run() will allow us to do things asynchronously
            io_context.run();

        }

    } catch (std::exception& err) {
        std::cerr << err.what() << std::endl;
    }

    return 0;
}
