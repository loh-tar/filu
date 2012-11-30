project(database)
#set(VERSION 0.1.0)

#  unset(DBUSER CACHE)
IF(NOT DBUSER)
    set(DBUSER filu CACHE STRING "Use only lower case letter.")
ENDIF(NOT DBUSER)
IF(NOT DBNAME)
    set(DBNAME filu CACHE STRING "Use only lower case letter.")
ENDIF(NOT DBNAME)

add_custom_target(init-filu
    # FIXME: Add checks if database and/or user already exist
    # psql postgres -tAc "SELECT 1 FROM pg_roles WHERE rolname='USR_NAME'" | grep -q 1 || createuser ...

    # Without option -P (prompt for password). Password is currently anyway not used
    createuser -e -D -R -S -U postgres ${DBUSER}
    COMMAND createdb -e -T template1 -U postgres -O ${DBUSER} ${DBNAME} "Filu - Supports your stock trading"
)

#
# Install all SQLs ...
set(SQL_PATH /usr/local/lib/Filu/sqls)
# ...Filu SQLs
install(DIRECTORY ./filu
        DESTINATION ${SQL_PATH}
)

# ...user SQLs. Rearrange them at the destination
install(DIRECTORY ./user/functions/
        DESTINATION ${SQL_PATH}/user/functions
)
install(DIRECTORY ./user/tables/
        DESTINATION ${SQL_PATH}/user/tables
)
install(DIRECTORY ./user/sqls/
        DESTINATION ${SQL_PATH}
)
