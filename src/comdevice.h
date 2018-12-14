/*
  Serial - TCP

  Mario Ban, 2018-02

*/

#ifndef COMDEVICE_H
#define COMDEVICE_H

#include <QObject>
#include <QByteArray>
#include <QSerialPort>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSocketNotifier>
#include <QFile>

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

// -------------------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------------------------------------

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

#endif // COMDEVICE_H
