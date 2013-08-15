add_subdirectory(agentf)
add_subdirectory(managerf)
add_subdirectory(performerf)
add_subdirectory(inspectorf)

install(PROGRAMS filu-cfg-postgresql
        DESTINATION bin
)
