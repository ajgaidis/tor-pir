#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include "pir.hpp"
#include "pir_client.hpp"
#include <seal/seal.h>

using boost::asio::ip::tcp;
namespace aio = boost::asio;
namespace po = boost::program_options;

#define DEFAULT_HOST ("localhost")
#define DEFAULT_PORT ("8080")
#define DEFAULT_ELE_NUM (4096)
#define DEFAULT_ELE_SIZE (288)  // Measured in bytes
#define DEFAULT_N (2048)
#define DEFAULT_LOGT (8)
#define DEFAULT_D (1)

std::string host;
std::string port;
uint64_t number_of_items;  // in the database
uint64_t size_per_item;    // measured in bytes
uint32_t N;                // polynomial degree of the ring
uint32_t logt;             // expansion factor (security parameter)
uint32_t d;                // dimension of the database


int main(int argc, char **argv) {

    // Use this to write timing data out to a log file specified by the path
//    std::cout << "[+] Opening a file to write to..." << std::flush;
//    std::string file = "/Users/agaidis/Desktop/tor-pir/logs/1-hop-latency.txt";
//    ofstream llog;
//    llog.open(file, fstream::out | fstream::app);
//    if (!llog.is_open()) {
//        std::cout << "[FAILURE]" << std::endl;
//    } else {
//        std::cout << "[SUCCESS]" << std::endl;
//    }

    // Define the options the program can take
    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "display help menu")
            ("pir","set PIR-based server")
            ("host,h", po::value<std::string>(&host)->default_value(DEFAULT_HOST),
             "set host to connect to")
            ("port,p", po::value<std::string>(&port)->default_value(DEFAULT_PORT),
             "set port number to connect to")
            ("ele_num", po::value<uint64_t>(&number_of_items)->default_value(DEFAULT_ELE_NUM),
             "set number of items in the database")
            ("ele_size", po::value<uint64_t>(&size_per_item)->default_value(DEFAULT_ELE_SIZE),
             "set the size of the database elements")
            ("N", po::value<uint32_t>(&N)->default_value(DEFAULT_N),
             "(PIR ONLY) set the polynomial degree of the ring")
            ("logt", po::value<uint32_t>(&logt)->default_value(DEFAULT_LOGT),
             "(PIR ONLY) set the security parameter")
            ("d", po::value<uint32_t>(&d)->default_value(DEFAULT_D),
             "(PIR ONLY) set the dimensions of the database")
            ;

    // Allow positional arguments
    po::positional_options_description p;
    // second argument below specifies the number of position arguments that will take this name
    p.add("host", 1);
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

    // Display the help menu
    if (vm.count("help")) {
        std::cout << "Usage:\n\t./client [options]\n" << desc << std::endl;
        return 1;
    }

    // Connect to a host on a given port
    std::cout << "[+] Connecting to " << host << ":" << port << "..." << std::flush;
    aio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver::query query(host, port, aio::ip::resolver_query_base::numeric_service);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    tcp::socket socket(io_context);
    aio::connect(socket, iterator);
    std::cout << "[SUCCESS]" << std::endl;

    std::string delimiter = ":q!";   // Used for async_read_until() to detect end of transmission
    std::random_device rd;           // Used to randomly select an element of the database to query

    if (vm.count("pir")) {

        /*
         * PIR CLIENT
         */
        seal::EncryptionParameters params(seal::scheme_type::BFV);
        PirParams pir_params;

        std::cout << "[+] Generating PIR parameters..." << std::flush;
        gen_params(number_of_items, size_per_item, N, logt, d, params, pir_params);
        std::cout << "[SUCCESS]" << std::endl;

        std::cout << "[+] Initializing PIR client..." << std::flush;
        PIRClient client(params, pir_params);
        std::cout << "[SUCCESS]" << std::endl;

        std::cout << "[+] Creating galois keys..." << std::flush;
        seal::GaloisKeys galois_keys = client.generate_galois_keys();
        std::cout << "[SUCCESS]" << std::endl;

        std::cout << "[+] Sending galois keys to server..." << std::flush;
        std::string serialized_galkeys = serialize_galoiskeys(galois_keys);
        // Append the delimiter to know when all the keys have been read into the buffer
        serialized_galkeys.append(delimiter);
        socket.write_some(aio::buffer(serialized_galkeys, serialized_galkeys.size()));
        // We sleep here to ensure the server is completely setup before hitting it with requests
        sleep(3);
        std::cout << "[SUCCESS]" << std::endl;

        int j = 0;
        while (j++ < 10) {  // We loop to take the average over multiple requests

            // The element to retreive and additional setup
            uint64_t ele_index = rd() % number_of_items;
            uint64_t index = client.get_fv_index(ele_index, size_per_item);
            // uint64_t offset = client.get_fv_offset(ele_index, size_per_item);

            // Timer to measure round-trip latency begins now
            auto time_before_pir = std::chrono::high_resolution_clock::now();

            // Generate the query from the element index
            PirQuery q = client.generate_query(index);
            std::string serialized_query = serialize_query(q);
            serialized_query.append(delimiter);

            // Write the query out to the server
            socket.write_some(aio::buffer(serialized_query, serialized_query.size()));
            // Read the response to the query from the server
            std::string response;
            std::size_t n = aio::read_until(socket, aio::dynamic_buffer(response), delimiter);
            std::string reply_str = response.substr(0, n);
            response.erase(0, n);

            reply_str.erase(reply_str.length() - delimiter.size(), delimiter.size());

            // Deserialize and decode the response from the server to get the answer
            PirReply reply = deserialize_ciphertexts(1, reply_str, CIPHER_SIZE);
            seal::Plaintext result = client.decode_reply(reply);

            vector<uint8_t> elems(N * logt / 8);
            coeffs_to_bytes(logt, result, elems.data(), (N * logt) / 8);

            // At this point we have a human-readable response and we can stop the clock
            auto time_after_pir = std::chrono::high_resolution_clock::now();
            auto time_difference_pir = std::chrono::duration_cast<std::chrono::microseconds>(
                    time_after_pir - time_before_pir).count();
            std::cout << "[i] Server response time: " << time_difference_pir << " microseconds" << std::endl;
            // std::cout << elems.data() << std::endl;
//            llog << time_difference_pir << "," << std::flush;
        }

    } else {

        /*
         * PLAIN CLIENT
         */

        uint64_t time_aggregate = 0;
        int i = 0;
        while (i++ < 100) {    // We loop to take the average over multiple requests

            // Generate a query
            uint64_t ele_index = rd() % number_of_items;
            std::string q = std::to_string(ele_index);
            q.append("\n");

            // Timer to measure round-trip latency begins now
            auto time_before_plain = std::chrono::high_resolution_clock::now();
            // Send the query out to the server
            socket.write_some(aio::buffer(q, q.size() + 1));

            // Read the reply from the server
            std::string response;
            std::size_t n = aio::read_until(socket, aio::dynamic_buffer(response), delimiter);
            std::string reply_str = response.substr(0, n);
            response.erase(0, n);
            reply_str.erase(reply_str.length() - delimiter.size(), delimiter.size());

            // At this point we have a human-readable response and we can stop the clock
            auto time_after_plain = std::chrono::high_resolution_clock::now();
            auto time_difference_plain = std::chrono::duration_cast<std::chrono::microseconds>(
                    time_after_plain - time_before_plain).count();
            std::cout << "Server response time: " << time_difference_plain << " microseconds" << std::endl;
            // llog << time_difference_plain << "," << std::flush;
            if (i != 1) // TODO: find out why this first query time is such an outlier
                time_aggregate += time_difference_plain;
        }
//        llog << time_aggregate / 100 << "," << std::flush;
    }

//    llog.close();
    return 0;
}