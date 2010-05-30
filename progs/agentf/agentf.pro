CONFIG += qt thread warn_on debug
QT += sql
#QMAKE_CXXFLAGS += -rdynamic

DESTDIR = ./../../tmp/bin
OBJECTS_DIR = ./../../tmp/agentf
MOC_DIR = $$OBJECTS_DIR

TEMPLATE = app
TARGET = agentf

DEPENDPATH += .
INCLUDEPATH += .

DEPENDPATH += ../../libs/database
DEPENDPATH += ../../libs/common

INCLUDEPATH += ../../libs/database
INCLUDEPATH += ../../libs/common

#LIBS += -L../../libs -lFilu
LIBS += -L./../../tmp/bin -lFilu

# Input
HEADERS += AgentF.h

SOURCES += main.cpp
SOURCES += AgentF.cpp

unix:LIBS += -lpq

target.path = /usr/local/bin
INSTALLS += target

#
# install provider scripts
#
Filu.path = /usr/local/lib/Filu/provider/Filu
Filu.files = provider/Filu/*.sh
Filu.files += provider/Filu/*.csv
INSTALLS += Filu

yahoo.path = /usr/local/lib/Filu/provider/Yahoo
yahoo.files += provider/Yahoo/*.pl
INSTALLS += yahoo

oanda.path = /usr/local/lib/Filu/provider/oanda
oanda.files = provider/oanda/*.pl
INSTALLS += oanda
