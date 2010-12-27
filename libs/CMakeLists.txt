add_subdirectory(core)
add_subdirectory(gui)

# Install ld.config file
set(LDCONFIG_FILES ./Filu.conf)
set(LDCONFIG_PATH /etc/ld.so.conf.d)
install(FILES ${LDCONFIG_FILES}
        DESTINATION ${LDCONFIG_PATH}
)
