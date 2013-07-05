add_subdirectory(core)
add_subdirectory(gui)

# Install ld.config file
set(LDCONFIG_FILES ./ld-config-file.conf)
set(LDCONFIG_PATH /etc/ld.so.conf.d)
install(FILES ${LDCONFIG_FILES}
        DESTINATION ${LDCONFIG_PATH}
        RENAME Filu.conf
)

# Install fixed SQL driver
# The permissions seems not be needed. It has also worked without
# these special settings. But any other lib has these permissions.
#
# Check if we have 32 or 64 bit machine
# http://www.cmake.org/pipermail/cmake/2011-June/045086.html
set(PSQLDRIVER ./libqsqlpsql.so-32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(PSQLDRIVER ./libqsqlpsql.so-64)
endif()

install(FILES ${PSQLDRIVER}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu/sqldrivers/
        PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
                    RENAME libqsqlpsql.so
)
