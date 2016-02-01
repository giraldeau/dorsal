#-------------------------------------------------
#
# Project created by QtCreator 2016-02-01T14:43:13
#
#-------------------------------------------------

QT       -= gui

TARGET = perf
TEMPLATE = lib

DEFINES += PERF_LIBRARY

SOURCES += perf.cpp \
    perfimpl.cpp \
    perfevent.cpp

HEADERS += perf.h\
        perf_global.h \
    perfimpl.h \
    perfevent.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
