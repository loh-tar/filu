CONFIG += qt thread warn_on debug
QT += sql
#QMAKE_CXXFLAGS += -rdynamic

DESTDIR = ./../../tmp/bin
OBJECTS_DIR = ./../../tmp/diggerf
MOC_DIR = $$OBJECTS_DIR

TEMPLATE = app
TARGET = diggerf

DEPENDPATH += .
INCLUDEPATH += .

DEPENDPATH += ../../libs/common
INCLUDEPATH += ../../libs/common

DEPENDPATH += ../../libs/database
INCLUDEPATH += ../../libs/database

DEPENDPATH += ../../libs/calc
INCLUDEPATH += ../../libs/calc

#LIBS += -L../../libs -lFilu
LIBS += -L./../../tmp/bin -lFilu

# TA-Lib stuff
INCLUDEPATH += /usr/local/include/ta-lib
LIBS += -L/usr/local/lib -lta_lib

# Input
HEADERS += DiggerF.h

SOURCES += main.cpp
SOURCES += DiggerF.cpp

unix:LIBS += -lpq

target.path = /usr/local/bin
INSTALLS += target

