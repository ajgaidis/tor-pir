cmake_minimum_required(VERSION 3.13)
project(tor_pir)

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.71.0 COMPONENTS system filesystem program_options REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(server server_main.cpp server_main.hpp tcp_server.cpp tcp_server.hpp tcp_connection.cpp tcp_connection.hpp)
    target_link_libraries(server ${Boost_LIBRARIES})
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
