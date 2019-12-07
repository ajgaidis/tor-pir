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
std::string delimiter = ":q!";

int main(int argc, char **argv) {

    try {

        if (argc != 4) {
            std::cerr << "Usage: client <host> <port> <plain | pir>" << std::endl;
        }

        aio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::query query(argv[1], argv[2], aio::ip::resolver_query_base::numeric_service);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket socket(io_context);
        aio::connect(socket, iterator);

        if (stoi(argv[3])) {
            /*
             * PIR CLIENT
             */
            uint64_t number_of_items = 4096;
            uint64_t size_per_item = 288; // in bytes
            uint32_t N = 2048;
            uint32_t logt = 8;
            uint32_t d = 1;
            std::random_device rd;
            seal::EncryptionParameters params(seal::scheme_type::BFV);
            PirParams pir_params;

            cout << "Generating all PIR parameters" << endl;
            gen_params(number_of_items, size_per_item, N, logt, d, params, pir_params);

            cout << "Initializing PIR client" << endl;
            PIRClient client(params, pir_params);
            seal::GaloisKeys galois_keys = client.generate_galois_keys();

            cout << "Sending Galois Keys to Server" << endl;
            std::string serialized_galkeys = serialize_galoiskeys(galois_keys);
            serialized_galkeys.append(delimiter); // Append the delimiter to know when all the keys have been read into the buffer
            socket.write_some(aio::buffer(serialized_galkeys, serialized_galkeys.size()));
//            boost::array<char, 42> buf{};
//            aio::read(socket, aio::buffer(buf, 42));
//            std::cout << buf.data() << std::endl;

            sleep(3);

            auto time_before_pir = std::chrono::high_resolution_clock::now();
            // Generate the query
            cout << "Generating the query" << endl;
            uint64_t ele_index = 25;
            uint64_t index = client.get_fv_index(ele_index, size_per_item);   // index of FV plaintext
            uint64_t offset = client.get_fv_offset(ele_index, size_per_item); // offset in FV plaintext
            PirQuery q = client.generate_query(index);
            std::cout << "Query size (# ciphertexts): " << q[0].size() << std::endl;
            std::string serialized_query = serialize_query(q);
            cout << "Serialized query size: " << serialized_query.length() << endl;
            serialized_query.append(delimiter);

            cout << "Writing the query out to the server" << endl;
            socket.write_some(aio::buffer(serialized_query, serialized_query.size()));
            boost::array<char, 32841> pir_buf{};

            cout << "Receiving response from the server" << endl;
            size_t len = aio::read(socket, aio::buffer(pir_buf, 32841));
            cout << "Deserializing response" << endl;

            //string reply_str = string((const char *)pir_buf.data(), 32841); // n will probably be CIPHER_SIZE * count
            std::string reply_str(pir_buf.begin(), pir_buf.end());
            cout << "Turned into string of size: " << reply_str.length() << endl;
//            cout << reply_str << endl;
            PirReply reply = deserialize_ciphertexts(1, reply_str, CIPHER_SIZE); // check the size of the ciphertext
            cout << "Decoding reply" << endl;
            seal::Plaintext result = client.decode_reply(reply);
            // Convert to elements
            cout << "Converting the response to bytes" << endl;
            vector<uint8_t> elems(N * logt / 8);
            coeffs_to_bytes(logt, result, elems.data(), (N * logt) / 8);
//
//            char buffer [1];
//            for (uint8_t i : elems) {
//                sprintf(buffer, "%02X", i);
//                std::cout << buffer << std::endl;
//            }

            auto time_after_pir = std::chrono::high_resolution_clock::now();
            auto time_difference_pir = std::chrono::duration_cast<std::chrono::microseconds>(
                    time_after_pir - time_before_pir).count();
            std::cout << "Server response time: " << time_difference_pir << " micoseconds" << std::endl;

            std::cout << elems.data() << std::endl;

        } else {
            /*
             * PLAIN CLIENT
             */
            auto time_before_plain = std::chrono::high_resolution_clock::now();
            socket.write_some(aio::buffer("5\n", 3));

            boost::array<char, 300> plain_buf{};
            boost::system::error_code plain_error;
            socket.read_some(aio::buffer(plain_buf), plain_error);

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