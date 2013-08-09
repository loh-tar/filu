#
# Install systemwide settings file
#

if(EXISTS "/etc/xdg/Filu.conf")
  file(INSTALL "${FiluRoot_SOURCE_DIR}/doc/systemwide-settings-file.conf"
       DESTINATION "/etc/xdg"
       RENAME "Filu.conf.new")
  message("*\n*  NOTICE: /etc/xdg/Filu.conf installed as Filu.conf.new\n*")

else()
  file(INSTALL "${FiluRoot_SOURCE_DIR}/doc/systemwide-settings-file.conf"
       DESTINATION "/etc/xdg"
       RENAME "Filu.conf")

endif()
