/*
  Serial - TCP

  Mario Ban, Atos AG, 02.2018

*/

#ifndef COMDEVICE_H
#define COMDEVICE_H

#include <QObject>

class ComDevice : public QObject
{
	Q_OBJECT

public:
	ComDevice(QObject *parent);
	~ComDevice();

private:
	
};

// -------------------------------------------------------------------------------------------------

class ComDeviceSerial : public ComDevice
{
	Q_OBJECT

public:
	ComDeviceSerial(const QString& serialPort, QObject *parent);
	~ComDeviceSerial();

public slots:
	void init();

private:
	const QString _serialPort;

};

// -------------------------------------------------------------------------------------------------

class ComDeviceTcp : public ComDevice
{
	Q_OBJECT

public:
	ComDeviceTcp(const QString& localIp, const QString& localPort, QObject *parent);
	~ComDeviceTcp();

public slots:
	void init();

private:
	const QString _localIp;
	const QString _localPort;

};

#endif // COMDEVICE_H
