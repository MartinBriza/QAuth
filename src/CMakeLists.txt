set(CMAKE_CXX_FLAGS "-g -Wall")
configure_file(common/config.h.in common/config.h IMMEDIATE @ONLY)
include_directories(common)
include_directories(${CMAKE_BINARY_DIR}/src/common)


set(QAuth_SRCS
    app/backend/PasswdBackend.cpp
    app/Backend.cpp
    app/QAuthApp.cpp
    app/Session.cpp
)

if(PAM_FOUND)
    set(QAuth_SRCS ${QAuth_SRCS}
        app/backend/PamHandle.cpp
        app/backend/PamBackend.cpp
    )
endif()

add_executable(QAuthHelper ${QAuth_SRCS})
target_link_libraries(QAuthHelper ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
if(PAM_FOUND)
    target_link_libraries(QAuthHelper ${PAM_LIBRARIES})
else()
    target_link_libraries(QAuthHelper crypt)
endif()

install(TARGETS QAuthHelper RUNTIME DESTINATION ${LIBEXEC_INSTALL_DIR})


set(libQAuth_SRCS
    lib/Prompt.cpp
    lib/QAuth.cpp
    lib/Request.cpp
)

add_library(QAuth SHARED ${libQAuth_SRCS})
target_link_libraries(QAuth ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS QAuth LIBRARY DESTINATION ${LIB_INSTALL_DIR})
install(FILES
    lib/QAuth
    lib/QAuth.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
