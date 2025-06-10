cmake_minimum_required(VERSION 3.16)
project(QtProjects)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64/lib/cmake")
set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(server)
add_subdirectory(client)