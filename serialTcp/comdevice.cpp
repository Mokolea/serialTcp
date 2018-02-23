/*
  Serial - TCP

  Mario Ban, Atos AG, 02.2018

*/

#include "comdevice.h"
#include <simpleQtLogger.h>

ComDevice::ComDevice(QObject *parent)
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
}

void ComDevice::slotDataSend(const QByteArray& data)
{
	L_FUNC("");
}

// -------------------------------------------------------------------------------------------------

ComDeviceSerial::ComDeviceSerial(const QString& serialPortName, QObject *parent)
	: ComDevice(parent)
	, _serialPortName(serialPortName)
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

	_serialPort = new QSerialPort(this);
	_serialPort->setPortName(_serialPortName);
	_serialPort->setBaudRate(QSerialPort::Baud9600);

	connect(_serialPort, &QSerialPort::readyRead, this, &ComDeviceSerial::slotReadyRead);
	connect(_serialPort, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &ComDeviceSerial::slotError);

	if (!_serialPort->open(QIODevice::ReadWrite)) {
		L_ERROR("Serial port open failed");
		emit finished();
	}
}

void ComDeviceSerial::slotDataSend(const QByteArray& data)
{
	L_FUNC("");
	if (!_serialPort) {
		return;
	}

	qint64 number = _serialPort->write(data);
	if(number == -1) {
		L_ERROR("Serial port write failed");
		emit finished();
	}
	else if(number != data.size()) {
		L_WARN("Serial port write partial data");
		emit finished();
	}
}

void ComDeviceSerial::slotReadyRead()
{
	L_FUNC("");
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
	L_WARN(QString("Serial port error: %1").arg(error));
	emit finished();
}

// -------------------------------------------------------------------------------------------------

ComDeviceTcp::ComDeviceTcp(const QString& localIp, const QString& localPort, QObject *parent)
	: ComDevice(parent)
	, _localIp(localIp)
	, _localPort(localPort)
	, _tcpServer(0)
{
	L_FUNC("");
}

ComDeviceTcp::~ComDeviceTcp()
{
	L_FUNC("");
	if (_tcpServer) {
		foreach(QTcpSocket* tcpSocket, _tcpSocketList) {
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

	if (!_tcpServer->listen(QHostAddress::Any, _localPort.toUShort())) {
		L_ERROR("TCP-Server listen failed");
		emit finished();
	}
}

void ComDeviceTcp::slotDataSend(const QByteArray& data)
{
	L_FUNC("");
	if (!_tcpServer) {
		return;
	}

	foreach(QTcpSocket* tcpSocket, _tcpSocketList) {
		qint64 number = tcpSocket->write(data);
		if (number == -1) {
			L_ERROR("TCP-Socket write failed");
			emit finished();
		}
		else if (number != data.size()) {
			L_WARN("TCP-Socket write partial data");
			emit finished();
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

	connect(tcpSocket, &QAbstractSocket::disconnected, this, &ComDeviceTcp::slotDisconnected);
	connect(tcpSocket, &QIODevice::readyRead, this, &ComDeviceTcp::slotReadyRead);

	_tcpSocketList << tcpSocket;

	L_NOTE("TCP-Socket connected");
}

void ComDeviceTcp::slotDisconnected()
{
	L_FUNC("");
	QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (!tcpSocket) {
		return;
	}
	_tcpSocketList.removeAll(tcpSocket);
	tcpSocket->deleteLater();
}

void ComDeviceTcp::slotReadyRead()
{
	L_FUNC("");
	QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (!tcpSocket) {
		return;
	}
	QByteArray data = tcpSocket->readAll();
	emit signalDataRecv(data);
}
