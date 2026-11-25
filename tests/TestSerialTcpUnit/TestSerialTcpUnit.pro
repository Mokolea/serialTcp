# Unit tests for serialTcp (Qt Test)

# https://github.com/Mokolea/serialTcp

QT += core serialport network testlib
QT -= gui

TARGET = TestSerialTcpUnit

CONFIG += c++11 console testcase
CONFIG -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS_DEBUG += -Wextra

INCLUDEPATH += \
  ../../src \
  ../../SimpleQtLogger

SOURCES += \
  tst_serialTcp.cpp \
  ../../src/task.cpp \
  ../../src/comdevice.cpp \
  ../../SimpleQtLogger/simpleQtLogger.cpp

HEADERS += \
  ../../src/task.h \
  ../../src/comdevice.h \
  ../../SimpleQtLogger/simpleQtLogger.h
