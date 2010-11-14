project(Filu)
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

set(Filu_SRC ${Filu_SRC}
    database/BarTuple.cpp
    database/Filu.cpp
    database/FiTuple.cpp
    database/QuoteTuple.cpp
    database/Tuple.cpp
    database/COTuple.cpp
    database/FiluU.cpp
    database/MarketTuple.cpp
    database/SymbolTuple.cpp
)
set(Filu_MOCH ${Filu_MOCH}
    database/FiTuple.h
    database/SymbolTuple.h
)
include_directories(database)

set(Filu_SRC ${Filu_SRC}
    common/DataTuple.cpp
    common/FClass.cpp
    common/FWidget.cpp
    common/RcFile.cpp
    common/SearchFiWidget.cpp
    common/DataTupleSet.cpp
    common/FObject.cpp
    common/Importer.cpp
    common/Script.cpp
    common/SqlTableView.cpp
    common/Exporter.cpp
    common/FTool.cpp
    common/LaunchPad.cpp
    common/SearchField.cpp
    common/StrategyEditor.cpp
    common/FToolBar.cpp
)
set(Filu_MOCH ${Filu_MOCH}
    common/LaunchPad.h
    common/Script.h
    common/SearchField.h
    common/SearchFiWidget.h
    common/SqlTableView.h
    common/StrategyEditor.h
    common/FToolBar.h
)
include_directories(common)

set(Filu_SRC ${Filu_SRC}
    calc/CalcBetween.cpp
    calc/CalcMuParser.cpp
    calc/CalcTALib.cpp
    calc/CalcWatchDogs.cpp
    calc/IndicatorSelector.cpp
    calc/Trader.cpp
    calc/CalcCross.cpp
    calc/CalcRef.cpp
    calc/CalcTrade.cpp
    calc/Indicator.cpp
    calc/MyParser.cpp
    calc/CalcFilu.cpp
    calc/CalcSetColor.cpp
    calc/CalcType.cpp
    calc/IndicatorEditor.cpp
    calc/TALib.cpp
)
set(Filu_MOCH ${Filu_MOCH}
    calc/IndicatorEditor.h
    calc/IndicatorSelector.h
)
include_directories(calc)

set(Filu_SRC ${Filu_SRC}
    graphics/COLine.cpp
    graphics/IndicatorPicker.cpp
    graphics/PlotHistogramBar.cpp
    graphics/Scaler.cpp
    graphics/COType.cpp
    graphics/IndicatorWidget.cpp
    graphics/PlotHistogram.cpp
    graphics/XScaleTicker.cpp
    graphics/Grip.cpp
    graphics/IndiWidgetGroup.cpp
    graphics/PlotLine.cpp
    graphics/IndicatorDataView.cpp
    graphics/PlotCandle.cpp
    graphics/PlotSheet.cpp
    graphics/IndicatorPainter.cpp
    graphics/PlotDashLine.cpp
    graphics/PlotType.cpp
)
set(Filu_MOCH ${Filu_MOCH}
    graphics/COType.h
    graphics/IndicatorDataView.h
    graphics/IndicatorPicker.h
    graphics/IndicatorWidget.h
    graphics/IndiWidgetGroup.h
    graphics/PlotSheet.h
)
include_directories(graphics)

QT4_WRAP_CPP(Filu_MOCSRC ${Filu_MOCH})
add_library(Filu SHARED ${Filu_SRC} ${Filu_MOCSRC})
set_target_properties(Filu PROPERTIES
    SOVERSION 0.1.0
)

target_link_libraries(Filu ${LIBS})
target_link_libraries(Filu ${QT_LIBRARIES})

install(TARGETS Filu
        LIBRARY DESTINATION lib/Filu
)

# Install ld.config file
set(LDCONFIG_FILES ./Filu.conf)
set(LDCONFIG_PATH /etc/ld.so.conf.d)
install(FILES ${LDCONFIG_FILES}
        DESTINATION ${LDCONFIG_PATH}
)

# Install SQLs
set(SQL_FILES ./database/sqls/)
set(SQL_PATH /usr/local/lib/Filu/sqls)
install(DIRECTORY ${SQL_FILES}
        DESTINATION ${SQL_PATH}
)
