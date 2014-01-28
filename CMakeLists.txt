project(QAuth)
cmake_minimum_required(VERSION 2.8.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt4 REQUIRED)
find_package(PAM)

include (InstallSettings)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wall -Wextra -std=c++11)

add_subdirectory(data)
add_subdirectory(example)
add_subdirectory(src)
add_subdirectory(test)