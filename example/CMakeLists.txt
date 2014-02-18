include_directories(${CMAKE_SOURCE_DIR}/src/lib)


set(checkpass_SRCS
    CheckPassApp.cpp
)

add_executable(checkpass ${checkpass_SRCS})
target_link_libraries(checkpass qauth ${QT_QTCORE_LIBRARY})


configure_file(minimaldm.service.in minimaldm.service IMMEDIATE @ONLY)

set(minimaldm_SRCS
    MinimalDMApp.cpp
)

add_executable(minimaldm ${minimaldm_SRCS})
target_link_libraries(minimaldm qauth ${QT_QTCORE_LIBRARY})

if (INSTALL_MINIMALDM)
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=systemdsystemunitdir systemd OUTPUT_VARIABLE SYSTEMD_UNIT_DIR)
    string(REGEX REPLACE "[ \t\n]+" \; SYSTEMD_UNIT_DIR "${SYSTEMD_UNIT_DIR}")
    if(NOT SYSTEMD_UNIT_DIR STREQUAL "")
        install(FILES minimaldm.service DESTINATION ${SYSTEMD_UNIT_DIR})
    endif()

    install(TARGETS minimaldm DESTINATION ${BIN_INSTALL_DIR})
endif()