project(inspectorf)

include_directories(${CMAKE_SOURCE_DIR} ${inspectorf_SOURCE_DIR} ${inspectorf_BINARY_DIR})

include_directories(${FiluCore_SOURCE_DIR})
include_directories(${FiluCore_SOURCE_DIR}/calc)
include_directories(${FiluCore_SOURCE_DIR}/database)

include_directories(${FiluGui_SOURCE_DIR})

set(inspectorf_SRC
    main.cpp
    InspectorF.cpp
    BackTester.cpp
)
set(inspectorf_MOCH
    BackTester.h
    InspectorF.h
)

QT4_WRAP_CPP(inspectorf_MOCSRC ${inspectorf_MOCH})

add_executable(inspectorf ${inspectorf_SRC} ${inspectorf_MOCSRC})
target_link_libraries(inspectorf FiluCore FiluGui ${QT_LIBRARIES})

install(TARGETS inspectorf
        RUNTIME DESTINATION bin
)

install(FILES filu-inspectorf.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES inspectorf.png DESTINATION ${ICON_INSTALL_DIR})
