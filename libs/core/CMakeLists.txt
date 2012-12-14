project(FiluCore)
#set(VERSION 0.1.0)
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

# CONFIG += thread warn_on debug
#CONFIG += create_prl

# TA-Lib stuff
find_package(ta_lib REQUIRED)
include_directories(${TA_LIB_INCLUDE_DIR})
set(LIBS ${LIBS} ${TA_LIB_LIBRARIES})

# muParser stuff
find_package(muparser REQUIRED)
include_directories(${MUPARSER_INCLUDE_DIR})
set(LIBS ${LIBS} ${MUPARSER_LIBRARIES})

set(FiluCore_SRC ${FiluCore_SRC}
    database/Tuple.cpp
    database/BarTuple.cpp
    database/BrokerTuple.cpp
    database/COTuple.cpp
    database/FiTuple.cpp
    database/MarketTuple.cpp
    database/QuoteTuple.cpp
    database/SymbolTuple.cpp
    database/Filu.cpp
    database/FiluU.cpp
)
set(FiluCore_MOCH ${FiluCore_MOCH}
    #database/Tuple.h
    #database/BarTuple.h
    #database/BrokerTuple.h
    #database/COTuple.h
    #database/FiTuple.h
    #database/MarketTuple.h
    #database/QuoteTuple.h
    #database/SymbolTuple.h
    #database/Filu.h
    #database/FiluU.h
)
include_directories(database)

set(FiluCore_SRC ${FiluCore_SRC}
    FTool.cpp
    RcFile.cpp
    Newswire.cpp
    FClass.cpp
    FObject.cpp
    FCoreApp.cpp
    CmdHelper.cpp
    DataTuple.cpp
    DataTupleSet.cpp
    Depots.cpp
    Exporter.cpp
    Importer.cpp
    Scanner.cpp
    Script.cpp
)
set(FiluCore_MOCH ${FiluCore_MOCH}
    #FTool.h
    #RcFile.h
    #Newswire.h
    #FClass.h
    #FObject.h
    #FCoreApp.h
    #DataTuple.h
    #DataTupleSet.h
    #Depots.h
    #Exporter.h
    #Importer.h
    #Scanner.h
    Script.h
)
include_directories(.)

set(FiluCore_SRC ${FiluCore_SRC}
    calc/MyParser.cpp
    calc/TALib.cpp
    calc/CalcType.cpp
    calc/CalcBetween.cpp
    calc/CalcCross.cpp
    calc/CalcFilu.cpp
    calc/CalcMuParser.cpp
    calc/CalcRef.cpp
    calc/CalcSetColor.cpp
    calc/CalcTALib.cpp
    calc/CalcTrade.cpp
    calc/CalcWatchDogs.cpp
    calc/Indicator.cpp
    calc/Trader.cpp
)
set(FiluCore_MOCH ${FiluCore_MOCH}
    #calc/MyParser.h
    #calc/TALib.h
    #calc/CalcType.h
    #calc/CalcBetween.h
    #calc/CalcCross.h
    #calc/CalcFilu.h
    #calc/CalcMuParser.h
    #calc/CalcRef.h
    #calc/CalcSetColor.h
    #calc/CalcTALib.h
    #calc/CalcTrade.h
    #calc/CalcWatchDogs.h
    #calc/Indicator.h
    #calc/Trader.h
)
include_directories(calc)
QT4_WRAP_CPP(FiluCore_MOCSRC ${FiluCore_MOCH})
add_library(FiluCore SHARED ${FiluCore_SRC} ${FiluCore_MOCSRC})
set_target_properties(FiluCore PROPERTIES
    SOVERSION 0.1.0
)

target_link_libraries(FiluCore ${LIBS} ${QT_LIBRARIES})

install(TARGETS FiluCore
        LIBRARY DESTINATION lib/Filu
)
