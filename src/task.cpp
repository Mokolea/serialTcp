/*

Serial - TCP

Mario Ban, 2018-02


MIT License

Copyright (c) 2018 Mario Ban

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

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
