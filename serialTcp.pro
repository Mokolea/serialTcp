#-------------------------------------------------
#
# Project created by QtCreator 2018-03-27T12:22:44
#
#-------------------------------------------------

QT += core serialport network
QT -= gui

TARGET = serialTcp

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += SimpleQtLogger

SOURCES += \
    serialTcp/main.cpp \
    serialTcp/task.cpp \
    serialTcp/comdevice.cpp \
    SimpleQtLogger/simpleQtLogger.cpp

HEADERS += \
    serialTcp/task.h \
    serialTcp/comdevice.h \
    SimpleQtLogger/simpleQtLogger.h
