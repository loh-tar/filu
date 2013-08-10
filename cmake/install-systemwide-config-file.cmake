#
# Install systemwide settings file
#

configure_file(${FiluRoot_SOURCE_DIR}/doc/systemwide-settings-file.conf
               ${FiluRoot_BINARY_DIR}
               @ONLY
)

if(EXISTS "/etc/xdg/Filu.conf")
  file(INSTALL "${FiluRoot_BINARY_DIR}/systemwide-settings-file.conf"
       DESTINATION "/etc/xdg"
       RENAME "Filu.conf.new")
  message("*\n*  NOTICE: /etc/xdg/Filu.conf installed as Filu.conf.new\n*")

else()
  file(INSTALL "${FiluRoot_BINARY_DIR}/systemwide-settings-file.conf"
       DESTINATION "/etc/xdg"
       RENAME "Filu.conf")

endif()
