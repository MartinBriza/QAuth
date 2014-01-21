
configure_file(common/config.h.in common/config.h IMMEDIATE @ONLY)
include_directories(common)
include_directories(${CMAKE_BINARY_DIR}/src/common)


set(QAuth_SRCS
    app/backend/PamHandle.cpp
    app/backend/PamBackend.cpp
    app/backend/PasswdBackend.cpp
    app/Backend.cpp
    app/QAuthApp.cpp
)
qt4_automoc(${QAuth_SRCS})

add_executable(QAuthHelper ${QAuth_SRCS})
target_link_libraries(QAuthHelper ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
if(PAM_FOUND)
    target_link_libraries(QAuthHelper ${PAM_LIBRARIES})
else()
    target_link_libraries(QAuthHelper crypt)
endif()

install(TARGETS QAuthHelper RUNTIME DESTINATION bin)


set(libQAuth_SRCS
    lib/QAuth.cpp
)
qt4_automoc(${libQAuth_SRCS})

add_library(QAuth SHARED ${libQAuth_SRCS})
target_link_libraries(QAuth ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})