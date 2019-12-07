#include <chrono>
#include <iostream>
#include <random>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "pir.hpp"
#include "pir_client.hpp"
#include <seal/seal.h>

using boost::asio::ip::tcp;
namespace aio = boost::asio;


int main(int argc, char **argv) {

    try {

        if (argc != 4) {
            std::cerr << "Usage: client <host> <port> <plain | pir>" << std::endl;
        }

        std::cout << "[+] Connecting to " << argv[1] << ":" << argv[2] << "..." << std::flush;
        aio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::query query(argv[1], argv[2], aio::ip::resolver_query_base::numeric_service);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket socket(io_context);
        aio::connect(socket, iterator);
        std::cout << "[SUCCESS]" << std::endl;


        if (stoi(argv[3])) {

            /*
             * PIR CLIENT
             */

            std::string delimiter = ":q!";   // Used for async_read_until() to detect end of transmission
            uint64_t number_of_items = 4096; // In the database
            uint64_t size_per_item = 288;    // Measured in bytes
            uint32_t N = 2048;
            uint32_t logt = 8;
            uint32_t d = 1;
            std::random_device rd;
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

            // The element to retreive and additional setup
            uint64_t ele_index = 25;
            uint64_t index = client.get_fv_index(ele_index, size_per_item);   // index of FV plaintext
            // uint64_t offset = client.get_fv_offset(ele_index, size_per_item); // offset in FV plaintext

QUERY_LOOP:
            // Timer to measure round-trip latency begins now
            auto time_before_pir = std::chrono::high_resolution_clock::now();

            // Generate the query from the element index
            PirQuery q = client.generate_query(index);
            std::string serialized_query = serialize_query(q);
            serialized_query.append(delimiter);

            // Write the query out to the server
            socket.write_some(aio::buffer(serialized_query, serialized_query.size()));

            // Read the response to the query from the server
            boost::array<char, 32841> pir_buf{};
            aio::read(socket, aio::buffer(pir_buf, 32841));
            std::string reply_str(pir_buf.begin(), pir_buf.end());

            // Deserialize and decode the response from the server to get the answer
            PirReply reply = deserialize_ciphertexts(1, reply_str, CIPHER_SIZE);
            seal::Plaintext result = client.decode_reply(reply);

            vector<uint8_t> elems(N * logt / 8);
            coeffs_to_bytes(logt, result, elems.data(), (N * logt) / 8);

            // At this point we have a human-readable response and we can stop the clock
            auto time_after_pir = std::chrono::high_resolution_clock::now();
            auto time_difference_pir = std::chrono::duration_cast<std::chrono::microseconds>(
                    time_after_pir - time_before_pir).count();
            std::cout << "[i] Server response time: " << time_difference_pir << " micoseconds" << std::endl;
            std::cout << elems.data() << std::endl;
            goto QUERY_LOOP;

        } else {

            /*
             * PLAIN CLIENT
             */

            // Timer to measure round-trip latency begins now
            auto time_before_plain = std::chrono::high_resolution_clock::now();
            // Send the query out to the server
            socket.write_some(aio::buffer("5\n", 3));

            // Read the reply from the server
            boost::array<char, 300> plain_buf{};
            boost::system::error_code plain_error;
            socket.read_some(aio::buffer(plain_buf), plain_error);

            // At this point we have a human-readable response and we can stop the clock
            auto time_after_plain = std::chrono::high_resolution_clock::now();
            auto time_difference_plain = std::chrono::duration_cast<std::chrono::microseconds>(
                    time_after_plain - time_before_plain).count();
            std::cout << "Server response time: " << time_difference_plain << " micoseconds" << std::endl;
        }

    } catch (std::exception& err) {
        std::cerr << err.what() << std::endl;
        return -1;
    }

    return 0;
}