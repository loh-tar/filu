project(managerf)

include_directories(${CMAKE_SOURCE_DIR} ${managerf_SOURCE_DIR} ${managerf_BINARY_DIR})

include_directories(${FiluCore_SOURCE_DIR})
include_directories(${FiluCore_SOURCE_DIR}/calc)
include_directories(${FiluCore_SOURCE_DIR}/database)

include_directories(${FiluGui_SOURCE_DIR})

set(managerf_SRC
    ManagerPage.cpp
    AddFiPage.cpp
    #ConfigPage.cpp
    FiPage.cpp
    IndicatorPage.cpp
    LogBookPage.cpp
    ManagerF.cpp
    main.cpp
)

set(managerf_MOCH
    ManagerPage.h
    AddFiPage.h
    FiPage.h
    IndicatorPage.h
    LogBookPage.h
    ManagerF.h
)

QT4_ADD_RESOURCES(managerf_MOCQRC icons.qrc)

QT4_WRAP_CPP(managerf_MOCSRC ${managerf_MOCH})

add_executable(managerf
               ${managerf_SRC}
               ${managerf_MOCSRC} ${managerf_MOCQRC}
)
target_link_libraries(managerf FiluCore FiluGui ${QT_LIBRARIES})

install(TARGETS managerf
        RUNTIME DESTINATION bin
)

install(FILES filu-managerf.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES managerf.png DESTINATION ${ICON_INSTALL_DIR})
