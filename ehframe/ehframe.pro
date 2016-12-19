QT += core
QT -= gui

TARGET = ehframe
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp


LIBS += -ldwarf -lelf
