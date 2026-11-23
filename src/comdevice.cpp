/*

Serial - TCP

Mario Ban, 2018-02
https://github.com/Mokolea/serialTcp


MIT License

Copyright (c) 2023 Mario Ban

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

#include "comdevice.h"
#include <simpleQtLogger.h>

#if COMDEVICE_ENABLE_TCP > 0
namespace {
// Limits per TCP client; the server accepts anyone, so a client must not grow memory without bound
const int TCP_LINE_SIZE_MAX = 64 * 1024;                 // incomplete line (no LF yet), Mode::TEXT
const qint64 TCP_SEND_BUFFER_SIZE_MAX = 4 * 1024 * 1024; // data not yet sent to a slow client
}
#endif

ComDevice::ComDevice(QObject* parent)
  : QObject(parent)
{
  L_FUNC("");
}

ComDevice::~ComDevice()
{
  L_FUNC("");
}

void ComDevice::init()
{
  L_FUNC("");
  L_FATAL("No implementation");
}

void ComDevice::slotDataSend(const QByteArray& data)
{
  L_FUNC("");
  L_FATAL("No implementation");
  Q_UNUSED(data);
}

// -------------------------------------------------------------------------------------------------

#if COMDEVICE_ENABLE_SERIAL > 0
ComDeviceSerial::ComDeviceSerial(const QString& serialPortName, const QString& serialBaudRate, QObject* parent)
  : ComDevice(parent)
  , _serialPortName(serialPortName)
  , _serialBaudRate(serialBaudRate)
  , _serialPort(0)
{
  L_FUNC("");
}

ComDeviceSerial::~ComDeviceSerial()
{
  L_FUNC("");
  if (_serialPort) {
    _serialPort->close();
    L_NOTE("Serial port closed");
  }
}

void ComDeviceSerial::init()
{
  L_FUNC("");

  bool ok = false;
  const qint32 baudRate = _serialBaudRate.toInt(&ok);
  if (!ok || baudRate <= 0) {
    L_ERROR(QString("Serial port baud rate invalid: %1").arg(_serialBaudRate));
    emit finished();
    return;
  }

  _serialPort = new QSerialPort(this);
  _serialPort->setPortName(_serialPortName);
  _serialPort->setBaudRate(baudRate);
  _serialPort->setDataBits(QSerialPort::Data8);
  _serialPort->setParity(QSerialPort::NoParity);
  _serialPort->setStopBits(QSerialPort::OneStop);
  _serialPort->setFlowControl(QSerialPort::NoFlowControl);

  connect(_serialPort, &QSerialPort::readyRead, this, &ComDeviceSerial::slotReadyRead);

  if (!_serialPort->open(QIODevice::ReadWrite)) {
    L_ERROR(QString("Serial port open failed: %1").arg(_serialPort->errorString()));
    emit finished();
    return;
  }

  // connect after open, so a failed open is reported once (above), not again by slotError()
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
  connect(_serialPort, &QSerialPort::errorOccurred, this, &ComDeviceSerial::slotError);
#else
  connect(_serialPort, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &ComDeviceSerial::slotError);
#endif
}

void ComDeviceSerial::slotDataSend(const QByteArray& data)
{
  // L_FUNC("");
  if (!_serialPort) {
    return;
  }

  qint64 number = _serialPort->write(data);
  if (number == -1) {
    L_ERROR("Serial port write failed");
    emit finished();
  }
  else if (number != data.size()) {
    L_WARN("Serial port write partial data");
    emit finished();
  }
}

void ComDeviceSerial::slotReadyRead()
{
  // L_FUNC("");
  if (!_serialPort) {
    return;
  }

  QByteArray data = _serialPort->readAll();
  emit signalDataRecv(data);
}

void ComDeviceSerial::slotError(QSerialPort::SerialPortError error)
{
  L_FUNC("");
  if (error == QSerialPort::NoError) {
    return;
  }
  L_WARN(QString("Serial port error: %1 (%2)").arg(error).arg(_serialPort ? _serialPort->errorString() : QString()));
  emit finished();
}
#endif

// -------------------------------------------------------------------------------------------------

#if COMDEVICE_ENABLE_TCP > 0
ComDeviceTcp::ComDeviceTcp(const QString& localIp, const QString& localPort, Mode mode, QObject* parent)
  : ComDevice(parent)
  , _localIp(localIp)
  , _localPort(localPort)
  , _mode(mode)
  , _tcpServer(0)
{
  L_FUNC("");
}

ComDeviceTcp::~ComDeviceTcp()
{
  L_FUNC("");
  if (_tcpServer) {
    foreach (QTcpSocket* tcpSocket, _tcpSocketList) {
      tcpSocket->disconnectFromHost();
      L_NOTE("TCP-Socket closed");
    }
    _tcpServer->close();
    L_NOTE("TCP-Server closed");
  }
}

void ComDeviceTcp::init()
{
  L_FUNC("");

  _tcpServer = new QTcpServer(this);

  connect(_tcpServer, &QTcpServer::acceptError, this, &ComDeviceTcp::slotAcceptError);
  connect(_tcpServer, &QTcpServer::newConnection, this, &ComDeviceTcp::slotNewConnection);

  QHostAddress hostAddress = _localIp.compare("any", Qt::CaseInsensitive) == 0 ? QHostAddress::Any : QHostAddress(_localIp);
  if (hostAddress.isNull()) {
    L_ERROR("TCP-Server listen address invalid");
    emit finished();
    return;
  }

  bool ok = false;
  quint16 localPort = _localPort.toUShort(&ok);
  if (!ok) {
    L_ERROR("TCP-Server local port invalid");
    emit finished();
    return;
  }

  if (!_tcpServer->listen(hostAddress, localPort)) {
    L_ERROR("TCP-Server listen failed");
    emit finished();
    return;
  }
  L_NOTE(QString("TCP-Server listening: %1 %2").arg(hostAddress.toString()).arg(_localPort));
}

void ComDeviceTcp::slotDataSend(const QByteArray& data)
{
  // L_FUNC("");
  if (!_tcpServer) {
    return;
  }

  // a failing or slow client is dropped, the others keep running
  const QList<QTcpSocket*> tcpSocketList = _tcpSocketList; // copy, dropClient() modifies the list
  for (QTcpSocket* tcpSocket : tcpSocketList) {
    qint64 number = tcpSocket->write(data);
    if (number == -1) {
      dropClient(tcpSocket, "write failed");
    }
    else if (number != data.size()) {
      dropClient(tcpSocket, "write partial data");
    }
    else if (tcpSocket->bytesToWrite() > TCP_SEND_BUFFER_SIZE_MAX) {
      dropClient(tcpSocket, "not reading, send buffer full");
    }
  }
}

void ComDeviceTcp::slotAcceptError(QAbstractSocket::SocketError socketError)
{
  L_FUNC("");
  L_ERROR(QString("TCP-Server accept error: %1").arg(socketError));
  emit finished();
}

void ComDeviceTcp::slotNewConnection()
{
  L_FUNC("");
  if (!_tcpServer) {
    return;
  }

  QTcpSocket* tcpSocket = _tcpServer->nextPendingConnection();
  if (!tcpSocket) {
    return;
  }

  if (_mode == Mode::BINARY && !_tcpSocketList.isEmpty()) {
    // limit to one connection
    L_NOTE(QString("TCP-Socket rejected, binary mode allows one connection: %1").arg(peerString(tcpSocket)));
    tcpSocket->abort();
    tcpSocket->deleteLater();
    return;
  }

  connect(tcpSocket, &QAbstractSocket::disconnected, this, &ComDeviceTcp::slotDisconnected);
  connect(tcpSocket, &QIODevice::readyRead, this, &ComDeviceTcp::slotReadyRead);

  _tcpSocketList << tcpSocket;

  L_NOTE(QString("TCP-Socket connected: %1 %2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort()));
}

void ComDeviceTcp::slotDisconnected()
{
  L_FUNC("");
  QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
  if (!tcpSocket) {
    return;
  }
  _dataRecv.remove(tcpSocket);
  _tcpSocketList.removeAll(tcpSocket);
  tcpSocket->deleteLater();
  L_NOTE(QString("TCP-Socket closed: %1 %2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort()));
}

void ComDeviceTcp::slotReadyRead()
{
  // L_FUNC("");
  QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
  if (!tcpSocket) {
    return;
  }
  QByteArray data = tcpSocket->readAll();
  if (_mode == Mode::BINARY) {
    emit signalDataRecv(data);
    return;
  }
  // Mode::TEXT: forward complete lines only, so lines of several clients do not interleave.
  // Work on bytes (not QString), so a UTF-8 character split across reads stays intact.
  QByteArray buffer = _dataRecv.take(tcpSocket) + data;
  auto index = buffer.indexOf('\n');
  while (index >= 0) {
    QByteArray line = buffer.left(index);
    buffer.remove(0, index + 1);
    if (line.endsWith('\r')) {
      line.chop(1); // CRLF -> LF
    }
    line.append('\n');
    emit signalDataRecv(line);
    index = buffer.indexOf('\n');
  }
  if (buffer.size() > TCP_LINE_SIZE_MAX) {
    dropClient(tcpSocket, "line too long");
    return;
  }
  if (!buffer.isEmpty()) {
    _dataRecv.insert(tcpSocket, buffer);
  }
}

QString ComDeviceTcp::peerString(const QTcpSocket* tcpSocket)
{
  // L_FUNC("");
  if (!tcpSocket) {
    return "-";
  }
  return QString("%1:%2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort());
}

void ComDeviceTcp::dropClient(QTcpSocket* tcpSocket, const QString& reason)
{
  L_FUNC("");
  L_WARN(QString("TCP-Socket dropped: %1 (%2)").arg(peerString(tcpSocket)).arg(reason));
  disconnect(tcpSocket, nullptr, this, nullptr); // no slotDisconnected(), cleanup is done here
  _dataRecv.remove(tcpSocket);
  _tcpSocketList.removeAll(tcpSocket);
  tcpSocket->abort();
  tcpSocket->deleteLater();
}
#endif

// -------------------------------------------------------------------------------------------------

#if COMDEVICE_ENABLE_SCREEN > 0
ComDeviceScreen::ComDeviceScreen(QObject* parent)
  : ComDevice(parent)
  , _fileOut(0)
{
  L_FUNC("");
}

ComDeviceScreen::~ComDeviceScreen()
{
  L_FUNC("");
}

void ComDeviceScreen::init()
{
  L_FUNC("");
  // local input (stdin) is not implemented

  // raw bytes, no text conversion: serial data may be binary or split inside a UTF-8 character
  _fileOut = new QFile(this);
  if (!_fileOut->open(stdout, QIODevice::WriteOnly | QIODevice::Unbuffered)) {
    L_ERROR(QString("Local output open failed: %1").arg(_fileOut->errorString()));
    emit finished();
  }
}

void ComDeviceScreen::slotDataSend(const QByteArray& data)
{
  // L_FUNC("");
  if (!_fileOut || !_fileOut->isOpen()) {
    return;
  }

  if (_fileOut->write(data) != data.size() || !_fileOut->flush()) {
    L_ERROR("Local output write failed");
    emit finished();
  }
}
#endif
