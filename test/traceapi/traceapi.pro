QT += core
QT -= gui

TARGET = traceapi
CONFIG += console
CONFIG -= app_bundle

CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp

LIBS += -llttng-ctl
