#
# Install systemwide config file
#

configure_file(${FiluRoot_SOURCE_DIR}/doc/systemwide-config-file.conf
               ${FiluRoot_BINARY_DIR}
               @ONLY
)

if(EXISTS "${Filu_XDG_DIRECTORY}/Filu.conf")
  file(INSTALL "${FiluRoot_BINARY_DIR}/systemwide-config-file.conf"
       DESTINATION "${Filu_XDG_DIRECTORY}"
       RENAME "Filu.conf.new")
  message("*\n*  NOTICE: ${Filu_XDG_DIRECTORY}/Filu.conf installed as Filu.conf.new\n*")

else()
  file(INSTALL "${FiluRoot_BINARY_DIR}/systemwide-config-file.conf"
       DESTINATION "${Filu_XDG_DIRECTORY}"
       RENAME "Filu.conf")

endif()
