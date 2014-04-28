project(QAuth)
cmake_minimum_required(VERSION 2.8.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(QAUTH_VERSION_X 0)
set(QAUTH_VERSION_Y 1)
set(QAUTH_VERSION_Z 0)
set(QAUTH_VERSION_STRING "${QAUTH_VERSION_X}.${QAUTH_VERSION_Y}.${QAUTH_VERSION_Z}")

option(BUILD_EXAMPLES "Builds the example applications using the library" OFF)
option(INSTALL_MINIMALDM "Installs the minimaldm example (you have to have it compiled, see BUILD_EXAMPLES)" OFF)
option(USE_QT5 "Uses Qt5 to compile the library" OFF)

if(USE_QT5)
    find_package(Qt5Core REQUIRED)
    set(COMPONENTS_VERSION 2.0)
else()
    find_package(Qt4 REQUIRED)
    set(COMPONENTS_VERSION 1.1)
endif()
find_package(PAM)

include (InstallSettings)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wall -Wextra -std=c++11)

configure_file(qauth.pc.in qauth.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qauth.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

add_subdirectory(data)
if (BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
add_subdirectory(src)
add_subdirectory(test)