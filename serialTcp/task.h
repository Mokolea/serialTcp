/*
  Serial - TCP

  Mario Ban, Atos AG, 02.2018

*/

#ifndef TASK_H
#define TASK_H

#include <QObject>
#include "comdevice.h"

class Task : public QObject
{
	Q_OBJECT

public:
	Task(const QString& serialPortName, const QString& serialBaudRate, const QString& localIp, const QString& localPort, QObject *parent = 0);
	~Task();

public slots:
	void init();

signals:
	void finished();

private:
	Q_DISABLE_COPY(Task)

private slots:
	void slotFinished();

private:
	const QString _serialPortName;
	const QString _serialBaudRate;
	const QString _localIp;
	const QString _localPort;

	ComDevice* _comDeviceSerial;
	ComDevice* _comDeviceTcp;
	ComDevice* _comDeviceScreen;
};

#endif // TASK_H
