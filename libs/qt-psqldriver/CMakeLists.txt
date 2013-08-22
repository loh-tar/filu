project(qsqlpsql)

set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

#
find_package(PostgreSQL REQUIRED)
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


#   include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}
# ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../core)
#
#   add_definitions(${QT_DEFINITIONS})
#   add_definitions(-DQT_PLUGIN)
#   add_definitions(-DQT_SHARED)
#   add_definitions(-DQT_NO_DEBUG)

#   qt4_wrap_cpp(myplugin_MOC_SRCS ${myplugin_MOC_HDRS})
#   add_library(myplugin SHARED ${myplugin_SRCS}
# ${myplugin_MOC_SRCS})
#   target_link_libraries(myplugin   ${QT_LIBRARIES}
# ${QT_QTXML_LIBRARY} mycore)



# install(FILES ${PSQLDRIVER}
#         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Filu/sqldrivers/
#         PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
#                     GROUP_EXECUTE GROUP_READ
#                     WORLD_EXECUTE WORLD_READ
#                     RENAME libqsqlpsql.so
# )
