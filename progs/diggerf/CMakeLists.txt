project(diggerf)
#CONFIG += thread warn_on debug
#QMAKE_CXXFLAGS += -rdynamic
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${diggerf_SOURCE_DIR} ${diggerf_BINARY_DIR})

include_directories(${FiluCore_SOURCE_DIR})
include_directories(${FiluCore_SOURCE_DIR}/calc)
include_directories(${FiluCore_SOURCE_DIR}/database)

set(diggerf_SRC
    main.cpp
    DiggerF.cpp
)

add_executable(diggerf ${diggerf_SRC})
target_link_libraries(diggerf FiluCore ${QT_LIBRARIES})

install(TARGETS diggerf
        RUNTIME DESTINATION bin
)
