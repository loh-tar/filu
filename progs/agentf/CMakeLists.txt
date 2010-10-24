project(agentf)
#CONFIG += thread warn_on debug
#QMAKE_CXXFLAGS += -rdynamic
set(QT_USE_QTSQL TRUE)

# This causes: ../FiluSource/libs/database/SymbolTuple.h:23:22: error: QTableView: No such file or directory
# So the lib sould be well redesigned that we can drop the
# QtGUI dependency
#set(QT_DONT_USE_QTGUI TRUE)

include(${QT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${agentf_SOURCE_DIR} ${agentf_BINARY_DIR})
include_directories( "${FiluRoot_SOURCE_DIR}/libs/database")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/common")

set(agentf_SRC AgentF.cpp  main.cpp)
QT4_WRAP_CPP(agentf_MOCSRCS AgentF.h)

add_executable(agentf ${agentf_SRC} ${agentf_MOCSRCS})

target_link_libraries(agentf Filu ${QT_LIBRARIES})

install(TARGETS agentf
        RUNTIME DESTINATION bin
)

#
# Install provider scripts
#
set(PROVIDER_FILES ${agentf_SOURCE_DIR}/provider/)
set(PROVIDER_PATH /usr/local/lib/Filu/provider)
install(DIRECTORY ${PROVIDER_FILES}
        DESTINATION ${PROVIDER_PATH}
)
