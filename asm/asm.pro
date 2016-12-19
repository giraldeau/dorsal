QT += core
QT -= gui

TARGET = asm
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += \
    ops.S \
    main.cpp

HEADERS += \
    ops.h

LIBS += -lunwind

QMAKE_CXXFLAGS += -fomit-frame-pointer
