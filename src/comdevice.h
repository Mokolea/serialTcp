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

#ifndef COMDEVICE_H
#define COMDEVICE_H

#ifndef COMDEVICE_ENABLE_SERIAL
#  define COMDEVICE_ENABLE_SERIAL   1
#endif
#ifndef COMDEVICE_ENABLE_TCP
#  define COMDEVICE_ENABLE_TCP   1
#endif
#ifndef COMDEVICE_ENABLE_SCREEN
#  define COMDEVICE_ENABLE_SCREEN   1
#endif

#include <QObject>
#include <QByteArray>
#if COMDEVICE_ENABLE_SERIAL > 0
#include <QSerialPort>
#endif
#if COMDEVICE_ENABLE_TCP > 0
#include <QTcpServer>
#include <QTcpSocket>
#include <QSocketNotifier>
#endif
#if COMDEVICE_ENABLE_SCREEN > 0
#include <QFile>
#endif

class ComDevice : public QObject
{
  Q_OBJECT

public:
  ComDevice(QObject *parent);
  virtual ~ComDevice();

public slots:
  virtual void init();
  virtual void slotDataSend(const QByteArray& data);

signals:
  void finished();
  void signalDataRecv(const QByteArray& data);

private:
  Q_DISABLE_COPY(ComDevice)

private:

};

// -------------------------------------------------------------------------------------------------

#if COMDEVICE_ENABLE_SERIAL > 0
class ComDeviceSerial : public ComDevice
{
  Q_OBJECT

public:
  explicit ComDeviceSerial(const QString& serialPortName, const QString& serialBaudRate, QObject *parent);
  virtual ~ComDeviceSerial();

public slots:
  virtual void init();
  virtual void slotDataSend(const QByteArray& data);

  void slotReadyRead();
  void slotError(QSerialPort::SerialPortError error);

private:
  Q_DISABLE_COPY(ComDeviceSerial)

private:
  const QString _serialPortName;
  const QString _serialBaudRate;

  QSerialPort* _serialPort;
};
#endif

// -------------------------------------------------------------------------------------------------

#if COMDEVICE_ENABLE_TCP > 0
class ComDeviceTcp : public ComDevice
{
  Q_OBJECT

public:
  ComDeviceTcp(const QString& localIp, const QString& localPort, QObject *parent);
  virtual ~ComDeviceTcp();

public slots:
  virtual void init();
  virtual void slotDataSend(const QByteArray& data);

  void slotAcceptError(QAbstractSocket::SocketError socketError);
  void slotNewConnection();
  void slotDisconnected();
  void slotReadyRead();

private:
  Q_DISABLE_COPY(ComDeviceTcp)

private:
  const QString _localIp;
  const QString _localPort;

  QTcpServer* _tcpServer;
  QList<QTcpSocket*> _tcpSocketList;
};
#endif

// -------------------------------------------------------------------------------------------------

#if COMDEVICE_ENABLE_SCREEN > 0
class ComDeviceScreen : public ComDevice
{
  Q_OBJECT

public:
  ComDeviceScreen(QObject *parent);
  virtual ~ComDeviceScreen();

public slots:
  virtual void init();
  virtual void slotDataSend(const QByteArray& data);

  void slotReadyRead();
  void slotActivated(int socket);

private:
  Q_DISABLE_COPY(ComDeviceScreen)

private:
  QTextStream* _textStreamIn;
  QTextStream* _textStreamOut;
  QSocketNotifier* _socketNotifierIn;
  QFile* _fileIn;
};
#endif

#endif // COMDEVICE_H
