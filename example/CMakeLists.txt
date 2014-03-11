include_directories(${CMAKE_SOURCE_DIR}/src/lib)



set(checkpass_SRCS
    CheckPassApp.cpp
)

add_executable(checkpass ${checkpass_SRCS})
if (USE_QT5)
    qt5_use_modules(checkpass Core Qml)
else()
    target_link_libraries(checkpass ${QT_QTCORE_LIBRARY})
endif()
target_link_libraries(checkpass qauth)



configure_file(minimaldm.service.in minimaldm.service IMMEDIATE @ONLY)

set(minimaldm_SRCS
    MinimalDMApp.cpp
)

add_executable(minimaldm ${minimaldm_SRCS})
if (USE_QT5)
    qt5_use_modules(minimaldm Core Qml)
else()
    target_link_libraries(minimaldm ${QT_QTCORE_LIBRARY})
endif()
target_link_libraries(minimaldm qauth)

if (INSTALL_MINIMALDM)
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=systemdsystemunitdir systemd OUTPUT_VARIABLE SYSTEMD_UNIT_DIR)
    string(REGEX REPLACE "[ \t\n]+" \; SYSTEMD_UNIT_DIR "${SYSTEMD_UNIT_DIR}")
    if(NOT SYSTEMD_UNIT_DIR STREQUAL "")
        install(FILES minimaldm.service DESTINATION ${SYSTEMD_UNIT_DIR})
    endif()

    install(TARGETS minimaldm DESTINATION ${BIN_INSTALL_DIR})
endif()



set(qmlapp_SRCS
    QMLApp.cpp
)

configure_file(qmlapp.qml qmlapp.qml)

add_executable(qmlapp ${qmlapp_SRCS})
if (USE_QT5)
    qt5_use_modules(qmlapp Core Qml Gui Quick)
else()
    target_link_libraries(qmlapp ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
endif()
target_link_libraries(qmlapp qauth)