/*
  Serial - TCP

  Mario Ban, Atos AG, 02.2018

*/

#include "task.h"
#include <simpleQtLogger.h>
#include <QtCore/QTimer>

Task::Task(const QString& serialPortName, const QString& localIp, const QString& localPort, QObject *parent)
	: QObject(parent)
	, _serialPortName(serialPortName)
	, _localIp(localIp)
	, _localPort(localPort)
	, _comDeviceSerial(0)
	, _comDeviceTcp(0)
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

	_comDeviceSerial = new ComDeviceSerial(_serialPortName, this);
	_comDeviceTcp = new ComDeviceTcp(_localIp, _localPort, this);

	connect(_comDeviceSerial, &ComDevice::finished, this, &Task::slotFinished);
	connect(_comDeviceTcp, &ComDevice::finished, this, &Task::slotFinished);

	connect(_comDeviceSerial, &ComDevice::signalDataRecv, _comDeviceTcp, &ComDevice::slotDataSend);
	connect(_comDeviceTcp, &ComDevice::signalDataRecv, _comDeviceSerial, &ComDevice::slotDataSend);

	QTimer::singleShot(0, _comDeviceSerial, SLOT(init()));
	QTimer::singleShot(0, _comDeviceTcp, SLOT(init()));
}

void Task::slotFinished()
{
	L_FUNC("");
	emit finished();
}
