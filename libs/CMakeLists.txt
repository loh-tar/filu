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
install(FILES ./libqsqlpsql.so
        DESTINATION /usr/local/lib/Filu/sqldrivers/
        PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
)
