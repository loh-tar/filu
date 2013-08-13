project (FiluRoot)
cmake_minimum_required (VERSION 2.8)
find_package(Qt4 REQUIRED)

# Disabled, it slows down the compilation
# But use it from time to time like valgrind
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
#set(CMAKE_CSS_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

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

#
# Install user default files
#
set(USERFILES_PATH ${CMAKE_INSTALL_PREFIX}/lib/Filu/userfiles)

install(DIRECTORY ${FiluRoot_SOURCE_DIR}/userfiles/
        DESTINATION ${USERFILES_PATH}
)
# Install example import files
install(DIRECTORY ${FiluRoot_SOURCE_DIR}/doc/
        DESTINATION ${USERFILES_PATH}/ImpFiles
        FILES_MATCHING PATTERN "*.imp"
)
# Install MakeNameNice.conf file
install(FILES ${FiluRoot_SOURCE_DIR}/doc/make-name-nice.conf
        DESTINATION ${USERFILES_PATH}
        RENAME MakeNameNice.conf
)

#
# Install user config files
#
install(FILES ${FiluRoot_SOURCE_DIR}/doc/user-config-file.conf
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu
)
# Install systemwide config file (it's tricky to avoid override)
configure_file(${FiluRoot_SOURCE_DIR}/cmake/install-systemwide-config-file.cmake
               ${FiluRoot_BINARY_DIR}
)
install(SCRIPT ${FiluRoot_BINARY_DIR}/install-systemwide-config-file.cmake)

#
# Install documentation
#
install(DIRECTORY ${FiluRoot_SOURCE_DIR}/doc/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/Filu
)
# Install readme file to documentation
install(FILES ${FiluRoot_SOURCE_DIR}/readme.txt
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/Filu
)

#
# Install provider scripts
#
install(DIRECTORY ${FiluRoot_SOURCE_DIR}/scripts/provider/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu/provider
        USE_SOURCE_PERMISSIONS
)

#
# Install perl modules
#
install(DIRECTORY ${FiluRoot_SOURCE_DIR}/scripts/perlmodules/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu/perlmodules
        USE_SOURCE_PERMISSIONS
)

# Unistall target
# For a more complex solution see
#   http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
add_custom_target(uninstall
    COMMAND echo "Going to remove Filu from the system..."
    COMMAND sudo xargs rm < install_manifest.txt
    COMMAND echo "Filu programs and files are now removed. The database is untouched."
)
