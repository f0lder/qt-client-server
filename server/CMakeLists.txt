cmake_minimum_required(VERSION 3.16)

project(ChatServer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64/lib/cmake")
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

add_executable(ChatServer
    main.cpp
    server.cpp
    server.h
)

target_link_libraries(ChatServer PRIVATE Qt6::Core Qt6::Network)
