add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(qt-psqldriver)

# Install ld.config file
install(FILES ./ld-config-file.conf
        DESTINATION /etc/ld.so.conf.d
        RENAME Filu.conf
)

