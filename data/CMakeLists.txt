if(PAM_FOUND)
    install(FILES
        pam/qauth-autologin
        pam/qauth-check
        pam/qauth-login
        DESTINATION
        ${SYSCONF_INSTALL_DIR}/pam.d
    )
endif()