project(inspectorf)
#CONFIG += thread warn_on debug
#QMAKE_CXXFLAGS += -rdynamic
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${inspectorf_SOURCE_DIR} ${inspectorf_BINARY_DIR})
include_directories( "${FiluRoot_SOURCE_DIR}/libs/common")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/database")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/calc")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/graphics")

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
target_link_libraries(inspectorf Filu ${QT_LIBRARIES})

install(TARGETS inspectorf
        RUNTIME DESTINATION bin
)