QT += core
QT -= gui

TARGET = defarg
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp \
    test.cpp \
    bidon.cpp

HEADERS += \
    test.h \
    bidon.h

