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
	Task(const QString& serialPort, const QString& localIp, const QString& localPort, QObject *parent = 0);
	~Task();

public slots:
	void init();

signals:
	void finished(); // TODO

private:
	Q_DISABLE_COPY(Task)

private:
	const QString _serialPort;
	const QString _localIp;
	const QString _localPort;

	ComDevice* _comDevice_1;
	ComDevice* _comDevice_2;

};

#endif // TASK_H
