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
	, _comDevice_1(0)
	, _comDevice_2(0)
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

	_comDevice_1 = new ComDeviceSerial(_serialPortName, this);
	_comDevice_2 = new ComDeviceTcp(_localIp, _localPort, this);

	connect(_comDevice_1, &ComDevice::finished, this, &Task::slotFinished);
	connect(_comDevice_2, &ComDevice::finished, this, &Task::slotFinished);

	connect(_comDevice_1, &ComDevice::signalDataRecv, _comDevice_2, &ComDevice::slotDataSend);
	connect(_comDevice_2, &ComDevice::signalDataRecv, _comDevice_1, &ComDevice::slotDataSend);

	QTimer::singleShot(0, _comDevice_1, SLOT(init()));
	QTimer::singleShot(0, _comDevice_2, SLOT(init()));
}

void Task::slotFinished()
{
	L_FUNC("");
	emit finished();
}
