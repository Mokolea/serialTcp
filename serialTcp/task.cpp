/*
  Serial - TCP

  Mario Ban, Atos AG, 02.2018

*/

#include "task.h"
#include <simpleQtLogger.h>
#include <QtCore/QTimer>

Task::Task(const QString& serialPort, const QString& localIp, const QString& localPort, QObject *parent)
	: QObject(parent)
	, _serialPort(serialPort)
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

	_comDevice_1 = new ComDeviceSerial(_serialPort, this);
	_comDevice_2 = new ComDeviceTcp(_localIp, _localPort, this);

	//connect(_tcRecv, SIGNAL(signalRFSI(const QString&, const QString&)), this, SLOT(slotRFSI(const QString&, const QString&)));
	//connect(_tcSend, SIGNAL(signalRFSI(const QString&, const QString&)), this, SLOT(slotRFSI(const QString&, const QString&)));

	//connect(_tcRecv, SIGNAL(signalDataRecv(const QString&, const QByteArray&)), this, SLOT(slotDataRecv(const QString&, const QByteArray&)));
	//connect(this, SIGNAL(signalDataSend(const QString&, const QByteArray&)), _tcSend, SLOT(slotDataSend(const QString&, const QByteArray&)));

	QTimer::singleShot(0, _comDevice_1, SLOT(init()));
	QTimer::singleShot(0, _comDevice_2, SLOT(init()));

	_sleep(1000);
	emit finished();
}
