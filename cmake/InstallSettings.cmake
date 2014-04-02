# Copyright (c) 2008 Kevin Krammer <kevin.krammer@gmx.at>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# this macro implements some very special logic how to deal with the cache
# by default the various install locations inherit their value from theit "parent" variable
# so if you set CMAKE_INSTALL_PREFIX, then EXEC_INSTALL_PREFIX, PLUGIN_INSTALL_DIR will
# calculate their value by appending subdirs to CMAKE_INSTALL_PREFIX
# this would work completely without using the cache.
# but if somebody wants e.g. a different EXEC_INSTALL_PREFIX this value has to go into
# the cache, otherwise it will be forgotten on the next cmake run.
# Once a variable is in the cache, it doesn't depend on its "parent" variables
# anymore and you can only change it by editing it directly.
# this macro helps in this regard, because as long as you don't set one of the
# variables explicitely to some location, it will always calculate its value from its
# parents. So modifying CMAKE_INSTALL_PREFIX later on will have the desired effect.
# But once you decide to set e.g. EXEC_INSTALL_PREFIX to some special location
# this will go into the cache and it will no longer depend on CMAKE_INSTALL_PREFIX.
macro(_SET_FANCY _var _value _comment)
    set(predefinedvalue "${_value}")

    if (NOT DEFINED ${_var})
        set(${_var} ${predefinedvalue})
    else (NOT DEFINED ${_var})
        set(${_var} "${${_var}}" CACHE PATH "${_comment}")
    endif (NOT DEFINED ${_var})
endmacro(_SET_FANCY)


_set_fancy(EXEC_INSTALL_PREFIX  "${CMAKE_INSTALL_PREFIX}"                 "Base directory for executables and libraries")
_set_fancy(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share"           "Base directory for files which go to share/")
_set_fancy(BIN_INSTALL_DIR      "${EXEC_INSTALL_PREFIX}/bin"              "The install dir for executables (default ${EXEC_INSTALL_PREFIX}/bin)")
_set_fancy(SBIN_INSTALL_DIR     "${EXEC_INSTALL_PREFIX}/sbin"             "The install dir for system executables (default ${EXEC_INSTALL_PREFIX}/sbin)")
_set_fancy(LIB_INSTALL_DIR      "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}" "The subdirectory relative to the install prefix where libraries will be installed (default is ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX})")
_set_fancy(LIBEXEC_INSTALL_DIR  "${CMAKE_INSTALL_PREFIX}/libexec"         "The subdirectory relative to the install prefix where libraries will be installed (default is ${CMAKE_INSTALL_PREFIX}/libexec)")
_set_fancy(INCLUDE_INSTALL_DIR  "${CMAKE_INSTALL_PREFIX}/include"         "The subdirectory to the header prefix")

_set_fancy(PLUGIN_INSTALL_DIR       "${LIB_INSTALL_DIR}"                "The subdirectory relative to the install prefix where plugins will be installed (default is ${LIB_INSTALL_DIR})")
_set_fancy(CONFIG_INSTALL_DIR       "${SHARE_INSTALL_PREFIX}/config"         "The config file install dir")
_set_fancy(DATA_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/apps"           "The parent directory where applications can install their data")
_set_fancy(HTML_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/doc/HTML"       "The HTML install dir for documentation")
_set_fancy(ICON_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/icons"          "The icon install dir (default ${SHARE_INSTALL_PREFIX}/share/icons/)")
_set_fancy(KCFG_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/config.kcfg"    "The install dir for kconfig files")
_set_fancy(LOCALE_INSTALL_DIR       "${SHARE_INSTALL_PREFIX}/locale"         "The install dir for translations")
_set_fancy(MIME_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/mimelnk"        "The install dir for the mimetype desktop files")
_set_fancy(SERVICES_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/services"  "The install dir for service (desktop, protocol, ...) files")
_set_fancy(SERVICETYPES_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/servicetypes" "The install dir for servicestypes desktop files")
_set_fancy(SOUND_INSTALL_DIR        "${SHARE_INSTALL_PREFIX}/sounds"         "The install dir for sound files")
_set_fancy(TEMPLATES_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/templates"      "The install dir for templates (Create new file...)")
_set_fancy(WALLPAPER_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/wallpapers"     "The install dir for wallpapers")
_set_fancy(DEMO_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/demos"          "The install dir for demos")
_set_fancy(KCONF_UPDATE_INSTALL_DIR "${DATA_INSTALL_DIR}/kconf_update"       "The kconf_update install dir")
_set_fancy(AUTOSTART_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/autostart"      "The install dir for autostart files")

_set_fancy(XDG_APPS_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/applications"         "The XDG apps dir")
_set_fancy(XDG_DIRECTORY_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/desktop-directories"      "The XDG directory")
_set_fancy(XDG_MIME_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/mime/packages"  "The install dir for the xdg mimetypes")

_set_fancy(SYSCONF_INSTALL_DIR      "/etc"            "The sysconfig install dir (default /etc)")
_set_fancy(MAN_INSTALL_DIR          "${SHARE_INSTALL_PREFIX}/man"            "The man install dir (default ${SHARE_INSTALL_PREFIX}/man/)")
_set_fancy(INFO_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/info"           "The info install dir (default ${SHARE_INSTALL_PREFIX}/info)")
_set_fancy(DBUS_INTERFACES_INSTALL_DIR      "${SHARE_INSTALL_PREFIX}/dbus-1/interfaces" "The dbus interfaces install dir (default  ${SHARE_INSTALL_PREFIX}/dbus-1/interfaces)")
_set_fancy(DBUS_SERVICES_INSTALL_DIR      "${SHARE_INSTALL_PREFIX}/dbus-1/services"     "The dbus services install dir (default  ${SHARE_INSTALL_PREFIX}/dbus-1/services)")

# The INSTALL_TARGETS_DEFAULT_ARGS variable should be used when libraries are installed.
# The arguments are also ok for regular executables, i.e. executables which don't go
# into sbin/ or libexec/, but for installing executables the basic syntax 
# INSTALL(TARGETS kate DESTINATION "${BIN_INSTALL_DIR}")
# is enough, so using this variable there doesn't help a lot.
# The variable must not be used for installing plugins.
# Usage is like this:
#    install(TARGETS kdecore kdeui ${INSTALL_TARGETS_DEFAULT_ARGS} )
#
# This will install libraries correctly under UNIX, OSX and Windows (i.e. dll's go
# into bin/.
# Later on it will be possible to extend this for installing OSX frameworks
# The COMPONENT Devel argument has the effect that static libraries belong to the 
# "Devel" install component. If we use this also for all install() commands
# for header files, it will be possible to install
#   -everything: make install OR cmake -P cmake_install.cmake
#   -only the development files: cmake -DCOMPONENT=Devel -P cmake_install.cmake
#   -everything except the development files: cmake -DCOMPONENT=Unspecified -P cmake_install.cmake
# This can then also be used for packaging with cpack.

set(INSTALL_TARGETS_DEFAULT_ARGS  RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
                                  LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                                  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}" COMPONENT Devel )


