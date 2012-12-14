project (FiluRoot)
cmake_minimum_required (VERSION 2.8)
find_package(Qt4 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# This setting was used at times where we need an own Qt lib with the fixed driver
# There was as option the entry..
#     /usr/local/Trolltech/Qt-4.3.5/lib
# ..in our /etc/ld.conf file
# nowday are both not needed, I hope
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(XDG_APPS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/applications CACHE FILEPATH "Install path for .desktop files")
# FIXME Here hard code the icon pfad because its also hard coded in the .desktop files
set(ICON_INSTALL_DIR /usr/share/icons/hicolor/48x48/apps/ CACHE FILEPATH "Global icon install path")
# set(ICON_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps CACHE FILEPATH "Global icon install path")

add_subdirectory(libs)
add_subdirectory(progs)
add_subdirectory(database)

# Install user default files
set(USERFILES_FILES ${FiluRoot_SOURCE_DIR}/userfiles/)
set(USERFILES_PATH ${CMAKE_INSTALL_PREFIX}/lib/Filu/userfiles)

install(DIRECTORY ${USERFILES_FILES}
        DESTINATION ${USERFILES_PATH}
)

# Install MakeNameNice.conf file
install(FILES ${FiluRoot_SOURCE_DIR}/doc/make-name-nice.conf
        DESTINATION ${USERFILES_PATH}
        RENAME MakeNameNice.conf
)

# Install user settings file
install(FILES ${FiluRoot_SOURCE_DIR}/doc/user-settings-file.conf
        DESTINATION ${USERFILES_PATH}
        RENAME Filu.conf
)

# Install systemwide settings file
set(SYSTEM_WIDE_SETTINGS_FILE Filu.conf)
if(EXISTS /etc/xdg/Filu.conf)
    set(SYSTEM_WIDE_SETTINGS_FILE Filu.conf.new)
endif(EXISTS /etc/xdg/Filu.conf)

install(FILES ${FiluRoot_SOURCE_DIR}/doc/systemwide-settings-file.conf
        DESTINATION /etc/xdg/
        RENAME ${SYSTEM_WIDE_SETTINGS_FILE}
)

#
# Install provider scripts
#
set(PROVIDER_FILES ${FiluRoot_SOURCE_DIR}/scripts/provider/)
set(PROVIDER_PATH ${CMAKE_INSTALL_PREFIX}/lib/Filu/provider)
install(DIRECTORY ${PROVIDER_FILES}
        USE_SOURCE_PERMISSIONS
        DESTINATION ${PROVIDER_PATH}
)

#
# Install perl modules
#
install(DIRECTORY ${FiluRoot_SOURCE_DIR}/scripts/perlmodules/
        USE_SOURCE_PERMISSIONS
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu/perlmodules
)

# Unistall target
# For a more complex solution see
#   http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
add_custom_target(uninstall
    COMMAND echo "Going to remove Filu from the system..."
    COMMAND sudo xargs rm < install_manifest.txt
    COMMAND echo "Filu programs and files are now removed. The database is untouched."
)
