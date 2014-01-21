include_directories(${CMAKE_SOURCE_DIR}/src/lib)

set(dataTransfer_SRCS
    DataTransfer.cpp
)
qt4_automoc(${QAuth_SRCS})

add_executable(dataTransfer ${dataTransfer_SRCS})
target_link_libraries(dataTransfer QAuth ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})


