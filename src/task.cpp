/*
  Serial - TCP

  Mario Ban, 2018-02

*/

#include "task.h"
#include <simpleQtLogger.h>
#include <QTimer>

Task::Task(const QString& serialPortName, const QString& serialBaudRate, const QString& localIp, const QString& localPort, bool localInput, bool localOutput, QObject *parent)
  : QObject(parent)
  , _serialPortName(serialPortName)
  , _serialBaudRate(serialBaudRate)
  , _localIp(localIp)
  , _localPort(localPort)
  , _localInput(localInput)
  , _localOutput(localOutput)
  , _comDeviceSerial(0)
  , _comDeviceTcp(0)
  , _comDeviceScreen(0)
{
  L_FUNC("");
}

Task::~Task()
{
  L_FUNC("");
}

void Task::init()
{
  L_FUNC("");

  _comDeviceSerial = new ComDeviceSerial(_serialPortName, _serialBaudRate, this);
  _comDeviceTcp = new ComDeviceTcp(_localIp, _localPort, this);
  _comDeviceScreen = new ComDeviceScreen(this);

  connect(_comDeviceSerial, &ComDevice::finished, this, &Task::slotFinished);
  connect(_comDeviceTcp, &ComDevice::finished, this, &Task::slotFinished);
  connect(_comDeviceScreen, &ComDevice::finished, this, &Task::slotFinished);

  connect(_comDeviceSerial, &ComDevice::signalDataRecv, _comDeviceTcp, &ComDevice::slotDataSend);
  connect(_comDeviceTcp, &ComDevice::signalDataRecv, _comDeviceSerial, &ComDevice::slotDataSend);
  if (_localOutput) {
    connect(_comDeviceSerial, &ComDevice::signalDataRecv, _comDeviceScreen, &ComDevice::slotDataSend);
  }
  if (_localInput) {
    connect(_comDeviceScreen, &ComDevice::signalDataRecv, _comDeviceSerial, &ComDevice::slotDataSend);
  }

  QTimer::singleShot(0, _comDeviceSerial, SLOT(init()));
  QTimer::singleShot(0, _comDeviceTcp, SLOT(init()));
  QTimer::singleShot(0, _comDeviceScreen, SLOT(init()));
}

void Task::slotFinished()
{
  L_FUNC("");
  emit finished();
}
