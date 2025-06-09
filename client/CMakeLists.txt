cmake_minimum_required(VERSION 3.16)

project(MyQtApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64/lib/cmake")
set(CMAKE_AUTOMOC ON)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Add executable using the WIN32 option to create a GUI application without a console window
add_executable(MyQtApp WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

# Link Qt libraries
target_link_libraries(MyQtApp Qt6::Widgets Qt6::Network)

target_include_directories(MyQtApp PRIVATE
    "C:/Qt/6.9.1/mingw_64/include"
    "C:/Qt/6.9.1/mingw_64/include/QtWidgets"
    "C:/Qt/6.9.1/mingw_64/include/QtCore"
    "C:/Qt/6.9.1/mingw_64/include/QtGui"
)
