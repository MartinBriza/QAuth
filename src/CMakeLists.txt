set(CMAKE_CXX_FLAGS "-g -Wall")
configure_file(common/config.h.in common/config.h IMMEDIATE @ONLY)
include_directories(common)
include_directories(${CMAKE_BINARY_DIR}/src/common)


set(Helper_SRCS
    app/backend/PasswdBackend.cpp
    app/Backend.cpp
    app/QAuthApp.cpp
    app/Session.cpp
    common/SafeDataStream.cpp
)

if(PAM_FOUND)
    set(Helper_SRCS ${Helper_SRCS}
        app/backend/PamHandle.cpp
        app/backend/PamBackend.cpp
    )
endif()

add_executable(qauthhelper ${Helper_SRCS})
if (USE_QT5)
    qt5_use_modules(qauthhelper Core Network)
else()
    target_link_libraries(qauthhelper ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
endif()
if(PAM_FOUND)
    target_link_libraries(qauthhelper ${PAM_LIBRARIES})
else()
    target_link_libraries(qauthhelper crypt)
endif()

install(TARGETS qauthhelper RUNTIME DESTINATION ${LIBEXEC_INSTALL_DIR})


set(libQAuth_SRCS
    lib/QAuth.cpp
    lib/QAuthPrompt.cpp
    lib/QAuthRequest.cpp
    common/SafeDataStream.cpp
)

add_library(qauth SHARED ${libQAuth_SRCS})
if (USE_QT5)
    qt5_use_modules(qauth Core Network Qml)
else()
    target_link_libraries(qauth ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY})
endif()

install(TARGETS qauth LIBRARY DESTINATION ${LIB_INSTALL_DIR})
install(FILES
    lib/QAuth
    lib/prompt.h
    lib/qauth.h
    lib/request.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/QAuth COMPONENT Devel)
