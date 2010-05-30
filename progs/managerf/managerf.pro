CONFIG += qt thread warn_on debug
QT += sql
#QMAKE_CXXFLAGS += -rdynamic

DESTDIR = ./../../tmp/bin
OBJECTS_DIR = ./../../tmp/managerf
MOC_DIR = $$OBJECTS_DIR

TEMPLATE = app
TARGET = managerf

DEPENDPATH += .
INCLUDEPATH += .

DEPENDPATH += ../../libs/common
INCLUDEPATH += ../../libs/common

DEPENDPATH += ../../libs/database
INCLUDEPATH += ../../libs/database

DEPENDPATH += ../../libs/calc
INCLUDEPATH += ../../libs/calc

DEPENDPATH += ../../libs/graphics
INCLUDEPATH += ../../libs/graphics

#LIBS += -L../../libs -lFilu
LIBS += -L./../../tmp/bin -lFilu

# TA-Lib stuff
INCLUDEPATH += /usr/local/include/ta-lib
LIBS += -L/usr/local/lib -lta_lib


# Input
#HEADERS += ConfigPage.h
HEADERS += FiPage.h
HEADERS += ManagerF.h
HEADERS += ManagerPage.h
HEADERS += IndicatorPage.h
HEADERS += AddFiPage.h

SOURCES += main.cpp
SOURCES += ManagerF.cpp
SOURCES += ManagerPage.cpp
#SOURCES += ConfigPage.cpp
SOURCES += FiPage.cpp
SOURCES += IndicatorPage.cpp
SOURCES += AddFiPage.cpp

#SOURCES += qrc_bilder.cpp

RESOURCES += icons.qrc

unix:LIBS += -lpq

target.path = /usr/local/bin
INSTALLS += target
