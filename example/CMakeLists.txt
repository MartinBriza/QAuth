include_directories(${CMAKE_SOURCE_DIR}/src/lib)


set(checkpass_SRCS
    CheckPassApp.cpp
)
qt4_automoc(${QAuth_SRCS})

add_executable(checkpass ${checkpass_SRCS})
target_link_libraries(checkpass QAuth ${QT_QTCORE_LIBRARY})


set(minimaldm_SRCS
    MinimalDMApp.cpp
)
qt4_automoc(${QAuth_SRCS})

add_executable(minimaldm ${minimaldm_SRCS})
target_link_libraries(minimaldm QAuth ${QT_QTCORE_LIBRARY})
