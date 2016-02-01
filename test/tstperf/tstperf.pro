#-------------------------------------------------
#
# Project created by QtCreator 2016-02-01T15:33:07
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_perftest
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += testcase

TEMPLATE = app


SOURCES += tst_perftest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(../../common.pri)
