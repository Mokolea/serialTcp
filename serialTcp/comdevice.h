/*
  Serial - TCP

  Mario Ban, Atos AG, 02.2018

*/

#ifndef COMDEVICE_H
#define COMDEVICE_H

#include <QObject>
#include <QByteArray>
#include <QSerialPort>
#include <QTcpServer>
#include <QTcpSocket>

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
	ComDeviceSerial(const QString& serialPortName, QObject *parent);
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

#endif // COMDEVICE_H
