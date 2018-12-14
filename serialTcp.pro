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
    src/main.cpp \
    src/task.cpp \
    src/comdevice.cpp \
    SimpleQtLogger/simpleQtLogger.cpp

HEADERS += \
    src/task.h \
    src/comdevice.h \
    SimpleQtLogger/simpleQtLogger.h
