project(FiluGui)
#set(VERSION 0.1.0)
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

# CONFIG += thread warn_on debug
#CONFIG += create_prl

include_directories(${FiluCore_SOURCE_DIR})
include_directories(${FiluCore_SOURCE_DIR}/calc)
include_directories(${FiluCore_SOURCE_DIR}/database)

set(FiluGui_SRC ${FiluGui_SRC}
    graphics/Grip.cpp
    graphics/COType.cpp
    graphics/COLine.cpp
    graphics/PlotType.cpp
    graphics/PlotCandle.cpp
    graphics/PlotDashLine.cpp
    graphics/PlotHistogramBar.cpp
    graphics/PlotHistogram.cpp
    graphics/PlotLine.cpp
    graphics/Scaler.cpp
    graphics/XScaleTicker.cpp
    graphics/PlotSheet.cpp
    graphics/IndicatorPainter.cpp
)
set(FiluGui_MOCH ${FiluGui_MOCH}
    #graphics/Grip.h
    graphics/COType.h
    #graphics/COLine.h
    #graphics/PlotType.h
    #graphics/PlotCandle.h
    #graphics/PlotDashLine.h
    #graphics/PlotHistogramBar.h
    #graphics/PlotHistogram.h
    #graphics/PlotLine.h
    #graphics/Scaler.h
    #graphics/XScaleTicker.h
    graphics/PlotSheet.h
    #graphics/IndicatorPainter.h
)
include_directories(graphics)

set(FiluGui_SRC ${FiluGui_SRC}
    FWidget.cpp
    ButtonPad.cpp
    DialogButton.cpp
    FiTableView.cpp
    FToolBar.cpp
    IndicatorDataView.cpp
    IndicatorEditor.cpp
    IndicatorPicker.cpp
    IndicatorSelector.cpp
    IndicatorWidget.cpp
    IndiSetPad.cpp
    IndiWidgetGroup.cpp
    IndiWidgetSimple.cpp
    LaunchPad.cpp
    LineEdit.cpp
    SearchField.cpp
    SearchFiWidget.cpp
    SqlTableView.cpp
    StrategyEditor.cpp
    SymbolTableView.cpp
)
set(FiluGui_MOCH ${FiluGui_MOCH}
    #FWidget.h
    ButtonPad.h
    DialogButton.h
    #FiTableView.h
    FToolBar.h
    IndicatorDataView.h
    IndicatorEditor.h
    IndicatorPicker.h
    IndicatorSelector.h
    IndicatorWidget.h
    IndiSetPad.h
    IndiWidgetGroup.h
    IndiWidgetSimple.h
    LaunchPad.h
    LineEdit.h
    SearchField.h
    SearchFiWidget.h
    SqlTableView.h
    StrategyEditor.h
    #SymbolTableView.h
)
include_directories(".")

QT4_WRAP_CPP(FiluGui_MOCSRC ${FiluGui_MOCH})
add_library(FiluGui SHARED ${FiluGui_SRC} ${FiluGui_MOCSRC})
set_target_properties(FiluGui PROPERTIES
    SOVERSION 0.1.0
)

target_link_libraries(FiluGui ${LIBS})
target_link_libraries(FiluGui ${QT_LIBRARIES})

install(TARGETS FiluGui
        LIBRARY DESTINATION lib/Filu
)
