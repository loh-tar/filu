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
    CmdHelper.cpp
    DataTuple.cpp
    DataTupleSet.cpp
    Depots.cpp
    Exporter.cpp
    FClass.cpp
    FObject.cpp
    FTool.cpp
    Importer.cpp
    Newswire.cpp
    RcFile.cpp
    Scanner.cpp
    Script.cpp
)
set(FiluCore_MOCH ${FiluCore_MOCH}
    #DataTuple.h
    #DataTupleSet.h
    #Depots.h
    #Exporter.h
    #FClass.h
    #FObject.h
    #FTool.h
    #Importer.h
    #Newswire.h
    #RcFile.h
    #Scanner.h
    Script.h
)
include_directories(.)

set(FiluCore_SRC ${FiluCore_SRC}
    calc/CalcBetween.cpp
    calc/CalcCross.cpp
    calc/CalcFilu.cpp
    calc/CalcMuParser.cpp
    calc/CalcRef.cpp
    calc/CalcSetColor.cpp
    calc/CalcTALib.cpp
    calc/CalcTrade.cpp
    calc/CalcType.cpp
    calc/CalcWatchDogs.cpp
    calc/Indicator.cpp
    calc/MyParser.cpp
    calc/TALib.cpp
    calc/Trader.cpp
)
set(FiluCore_MOCH ${FiluCore_MOCH}
    #calc/CalcBetween.h
    #calc/CalcCross.h
    #calc/CalcFilu.h
    #calc/CalcMuParser.h
    #calc/CalcRef.h
    #calc/CalcSetColor.h
    #calc/CalcTALib.h
    #calc/CalcTrade.h
    #calc/CalcType.h
    #calc/CalcWatchDogs.h
    #calc/Indicator.h
    #calc/MyParser.h
    #calc/TALib.h
    #calc/Trader.h
)
include_directories(calc)

set(FiluCore_SRC ${FiluCore_SRC}
    database/BarTuple.cpp
    #database/BrokerTuple.cpp
    database/COTuple.cpp
    database/Filu.cpp
    database/FiluU.cpp
    database/FiTuple.cpp
    database/MarketTuple.cpp
    database/QuoteTuple.cpp
    database/SymbolTuple.cpp
    database/Tuple.cpp
)
set(FiluCore_MOCH ${FiluCore_MOCH}
    #database/BarTuple.h
    #database/BrokerTuple.h
    #database/COTuple.h
    #database/Filu.h
    #database/FiluU.h
    #database/FiTuple.h
    #database/MarketTuple.h
    #database/QuoteTuple.h
    #database/SymbolTuple.h
    #database/Tuple.h
)
include_directories(database)

QT4_WRAP_CPP(FiluCore_MOCSRC ${FiluCore_MOCH})
add_library(FiluCore SHARED ${FiluCore_SRC} ${FiluCore_MOCSRC})
set_target_properties(FiluCore PROPERTIES
    SOVERSION 0.1.0
)

target_link_libraries(FiluCore ${LIBS} ${QT_LIBRARIES})

install(TARGETS FiluCore
        LIBRARY DESTINATION lib/Filu
)

# Install SQLs
set(SQL_FILES ./database/sqls/)
set(SQL_PATH /usr/local/lib/Filu/sqls)
install(DIRECTORY ${SQL_FILES}
        DESTINATION ${SQL_PATH}
)
