project(agentf)
#CONFIG += thread warn_on debug
#QMAKE_CXXFLAGS += -rdynamic
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${agentf_SOURCE_DIR} ${agentf_BINARY_DIR})

include_directories(${FiluCore_SOURCE_DIR})
include_directories(${FiluCore_SOURCE_DIR}/database)

set(agentf_SRC AgentF.cpp  main.cpp CmdAdd.cpp)
QT4_WRAP_CPP(agentf_MOCSRCS AgentF.h)

add_executable(agentf ${agentf_SRC} ${agentf_MOCSRCS})

target_link_libraries(agentf FiluCore ${QT_LIBRARIES})

install(TARGETS agentf
        RUNTIME DESTINATION bin
)

#
# Install provider scripts
#
set(PROVIDER_FILES ${agentf_SOURCE_DIR}/provider/)
set(PROVIDER_PATH /usr/local/lib/Filu/provider)
install(DIRECTORY ${PROVIDER_FILES}
        USE_SOURCE_PERMISSIONS
        DESTINATION ${PROVIDER_PATH}
)

#
# Install perl modules
#
install(DIRECTORY ${agentf_SOURCE_DIR}/perlmodules/
        USE_SOURCE_PERMISSIONS
        DESTINATION /usr/local/lib/Filu/perlmodules
)
