project(agentf)

include_directories(${CMAKE_SOURCE_DIR} ${agentf_SOURCE_DIR} ${agentf_BINARY_DIR})

include_directories(${FiluCore_SOURCE_DIR})
include_directories(${FiluCore_SOURCE_DIR}/database)

set(agentf_SRC
    CmdClass.cpp
    CmdAdd.cpp
    CmdDB.cpp
    CmdImp.cpp
    CmdSummon.cpp
    CmdExorcise.cpp
    CmdConfig.cpp
    CmdSplitBars.cpp
    CmdDeleteBars.cpp
    CmdThis.cpp
    CmdFetch.cpp
    CmdDo.cpp
    CmdDoc.cpp
    AgentF.cpp
    main.cpp
)

QT4_WRAP_CPP(agentf_MOCSRCS AgentF.h)

add_executable(agentf ${agentf_SRC} ${agentf_MOCSRCS})

target_link_libraries(agentf FiluCore ${QT_LIBRARIES})

install(TARGETS agentf
        RUNTIME DESTINATION bin
)
