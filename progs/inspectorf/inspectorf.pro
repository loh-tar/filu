CONFIG += qt thread warn_on debug
QT += sql
#QMAKE_CXXFLAGS += -rdynamic

DESTDIR = ./../../tmp/bin
OBJECTS_DIR = ./../../tmp/inspectorf
MOC_DIR = $$OBJECTS_DIR

TEMPLATE = app
TARGET = inspectorf

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
HEADERS += InspectorF.h
HEADERS += BackTester.h

SOURCES += main.cpp
SOURCES += InspectorF.cpp
SOURCES += BackTester.cpp

unix:LIBS += -lpq

target.path = /usr/local/bin
INSTALLS += target

