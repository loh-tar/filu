project(database)
#set(VERSION 0.1.0)

#  unset(DBUSER CACHE)
if(NOT DBUSER)
    set(DBUSER filu CACHE STRING "Use only lower case letter.")
endif(NOT DBUSER)

if(NOT DBNAME)
    set(DBNAME filu CACHE STRING "Use only lower case letter.")
endif(NOT DBNAME)

add_custom_target(init-filu
    # FIXME: Add checks if database and/or user already exist
    # psql postgres -tAc "SELECT 1 FROM pg_roles WHERE rolname='USR_NAME'" | grep -q 1 || createuser ...

    # Without option -P (prompt for password). Password is currently anyway not used
    COMMAND createuser -e -D -R -S -U postgres ${DBUSER}
    COMMAND createdb -e -T template1 -U postgres -O ${DBUSER} ${DBNAME} "Filu - Supports your stock trading"
)

#
# Install all SQLs
install(DIRECTORY ./sqls/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu/sqls
)
