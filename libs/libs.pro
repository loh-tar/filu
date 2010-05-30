CONFIG += qt thread warn_on debug
#CONFIG += create_prl
#CONFIG += staticlib

DESTDIR = ./../tmp/bin
OBJECTS_DIR = ./../tmp/libs
MOC_DIR = $$OBJECTS_DIR

QT += sql

TEMPLATE = lib
TARGET = Filu
VERSION = 0.1.0


DEPENDPATH += .
INCLUDEPATH += .

DEPENDPATH += ./database
INCLUDEPATH += ./database

DEPENDPATH += ./common
INCLUDEPATH += ./common

DEPENDPATH += ./calc
INCLUDEPATH += ./calc


DEPENDPATH += ./graphics
INCLUDEPATH += ./graphics

# TA-Lib stuff
INCLUDEPATH += /usr/local/include/ta-lib
LIBS += -L/usr/local/lib -lta_lib

# muParser stuff
INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib -lmuparser

unix:LIBS += -lpq

HEADERS = $$system(find -regex ".+\.h" -type f)
SOURCES = $$system(find -regex ".+\.cpp" -type f)

target.path = /usr/local/lib/Filu/
INSTALLS += target

#header.path = /usr/local/include/Filu/
#header.files = *.h
#INSTALLS += header

ldconfig.path = /etc/ld.so.conf.d
ldconfig.files = ./Filu.conf
INSTALLS += ldconfig

sql.path = /usr/local/lib/Filu/sqls/
sql.files = ./database/sqls/*.sql
INSTALLS += sql

