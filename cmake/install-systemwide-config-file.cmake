#
# Install systemwide config file
#

configure_file(${FiluRoot_SOURCE_DIR}/doc/systemwide-config-file.conf
               ${FiluRoot_BINARY_DIR}
               @ONLY
)

if(EXISTS "/etc/xdg/Filu.conf")
  file(INSTALL "${FiluRoot_BINARY_DIR}/systemwide-config-file.conf"
       DESTINATION "/etc/xdg"
       RENAME "Filu.conf.new")
  message("*\n*  NOTICE: /etc/xdg/Filu.conf installed as Filu.conf.new\n*")

else()
  file(INSTALL "${FiluRoot_BINARY_DIR}/systemwide-config-file.conf"
       DESTINATION "/etc/xdg"
       RENAME "Filu.conf")

endif()
