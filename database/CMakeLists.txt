project(database)
#set(VERSION 0.1.0)

#
# Install all SQLs
install(DIRECTORY ./sqls/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu/sqls
)
