#-------------------------------------------------
#
# Project created by QtCreator 2016-02-03T10:41:08
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_tasktest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_tasktest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(../../common.pri)
