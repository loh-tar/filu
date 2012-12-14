project(performerf)
#CONFIG += thread warn_on debug
#QMAKE_CXXFLAGS += -rdynamic
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${agentf_SOURCE_DIR} ${agentf_BINARY_DIR})

include_directories(${FiluCore_SOURCE_DIR})
include_directories(${FiluCore_SOURCE_DIR}/calc)
include_directories(${FiluCore_SOURCE_DIR}/database)

include_directories(${FiluGui_SOURCE_DIR})
include_directories(${FiluGui_SOURCE_DIR}/graphics)

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

target_link_libraries(performerf FiluCore FiluGui ${QT_LIBRARIES})

install(TARGETS performerf
        RUNTIME DESTINATION bin
)

install(FILES filu-performerf.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES performerf.png DESTINATION ${ICON_INSTALL_DIR})
