cmake_minimum_required(VERSION 3.10.2)
project(tor_pir)

# Use the Gnu C++ compiler
set(CMAKE_CXX_COMPILER "/user/bin/g++")
#set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-9")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS -fvisibility=hidden)

# This allowed certain later features std, like shared_mutex
add_definitions(-std=gnu++17)

# Send the compiled binary to the top-level bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

## Enable for verbosity when building
# set(CMAKE_VERBOSE_MAKEFILE ON)

# The main server executable
add_executable(server
        server_main.cpp
        )
# The main client executable
add_executable(client
        client_main.cpp
        )

# Link the custom server libraries we created
add_library(libserver STATIC
        tcp_server.cpp
        plain_server.cpp)
target_link_libraries(server libserver)

# Add boost libraries
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.71.0 COMPONENTS system filesystem program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(server ${Boost_LIBRARIES})
target_link_libraries(client ${Boost_LIBRARIES})

# Link the SealPIR files
add_library(sealpir STATIC
        pir.cpp
        pir_client.cpp
        pir_server.cpp
)
target_link_libraries(server sealpir)
target_link_libraries(client sealpir)

# Link SEAL
find_package(SEAL 3.2.0 EXACT REQUIRED)
target_link_libraries(server SEAL::seal)
target_link_libraries(client SEAL::seal)