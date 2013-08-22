project(qsqlpsql)

find_package(PostgreSQL ${ReqPostgreSqlVersion} REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIR})
set(LIBS ${LIBS} ${PostgreSQL_LIBRARIES})

set(qsqlpsql_SRC ${qsqlpsql_SRC}
    qsql_psql.cpp
    main.cpp
)

set(qsqlpsql_MOCH ${qsqlpsql_MOCH}
    qsql_psql.h
)

QT4_WRAP_CPP(qsqlpsql_MOCSRC ${qsqlpsql_MOCH})
add_library(qsqlpsql SHARED ${qsqlpsql_SRC} ${qsqlpsql_MOCSRC})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_NO_DEBUG)

set_target_properties(qsqlpsql PROPERTIES
    SOVERSION 0.1.0
)

target_link_libraries(qsqlpsql ${LIBS} ${QT_LIBRARIES})

install(TARGETS qsqlpsql
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu/sqldrivers/
)
