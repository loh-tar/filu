project(performerf)
#CONFIG += thread warn_on debug
#QMAKE_CXXFLAGS += -rdynamic

set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${agentf_SOURCE_DIR} ${agentf_BINARY_DIR})
include_directories( "${FiluRoot_SOURCE_DIR}/libs/common")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/database")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/calc")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/graphics")

set(performerf_SRC
    main.cpp
    PerformerF.cpp
    FiGroupWidget.cpp
)
set(performerf_MOCH
    PerformerF.h
    FiGroupWidget.h
    FiGroupWidget.h
)

QT4_WRAP_CPP(performerf_MOCSRC ${performerf_MOCH})

add_executable(performerf ${performerf_SRC} ${performerf_MOCSRC})

target_link_libraries(performerf Filu ${QT_LIBRARIES})

install(TARGETS performerf
        RUNTIME DESTINATION bin
)
