project(diggerf)
#CONFIG += thread warn_on debug
#QMAKE_CXXFLAGS += -rdynamic
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${diggerf_SOURCE_DIR} ${diggerf_BINARY_DIR})
include_directories( "${FiluRoot_SOURCE_DIR}/libs/common")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/database")
include_directories( "${FiluRoot_SOURCE_DIR}/libs/calc")

set(diggerf_SRC
    main.cpp
    DiggerF.cpp
)

add_executable(diggerf ${diggerf_SRC})
target_link_libraries(diggerf Filu ${QT_LIBRARIES})

install(TARGETS diggerf
        RUNTIME DESTINATION bin
)
