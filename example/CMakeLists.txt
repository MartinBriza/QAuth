include_directories(${CMAKE_SOURCE_DIR}/src/lib)


set(checkpass_SRCS
    CheckPassApp.cpp
)

add_executable(checkpass ${checkpass_SRCS})
target_link_libraries(checkpass QAuth ${QT_QTCORE_LIBRARY})


set(minimaldm_SRCS
    MinimalDMApp.cpp
)

add_executable(minimaldm ${minimaldm_SRCS})
target_link_libraries(minimaldm QAuth ${QT_QTCORE_LIBRARY})

install(TARGETS minimaldm DESTINATION ${BIN_INSTALL_DIR})