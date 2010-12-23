project (FiluRoot)
cmake_minimum_required (VERSION 2.8)
find_package(Qt4 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# This setting is controversial. But without we got a linker runtime error
# when use the self compiled Qt library.
# An other solution is to include the line
#   /usr/local/Trolltech/Qt-4.3.5/lib
# to FiluSource/libs/Filu.conf. Let me know what is the right way.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# When we use ...
# set(XDG_APPS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/applications CACHE FILEPATH "Install path for .desktop files")
# set(ICON_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/pixmaps CACHE FILEPATH "Global icon install path")
# ...will the *.desktop files and icons not found (!?) so we hard code:
set(XDG_APPS_INSTALL_DIR /usr/share/applications CACHE FILEPATH "Install path for .desktop files")
set(ICON_INSTALL_DIR /usr/share/pixmaps CACHE FILEPATH "Global icon install path")

add_subdirectory(libs)
add_subdirectory(progs)

# Install user default files
set(USERFILES_FILES ${FiluRoot_SOURCE_DIR}/userfiles/)
set(USERFILES_PATH /usr/local/lib/Filu/userfiles)

install(DIRECTORY ${USERFILES_FILES}
        DESTINATION ${USERFILES_PATH}
)
