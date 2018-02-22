/*
  Serial - TCP

	Mario Ban, Atos AG, 02.2018

*/

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include "task.h"

#define TOOL_VERSION "1.0"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if (argc < 4) {
		qWarning("ERROR: missing command-line parameter");
		qWarning("");
		qWarning("Usage: %s serialPort localIp localPort", a.arguments().at(0).toStdString().c_str());
		qWarning("  serialPort serial port this program is opening");
		qWarning("  localIp IP-address this program is binding to");
		qWarning("  localPort port used by this program, listening");
		exit(1);
	}

	qDebug() << argv[0] << "v" TOOL_VERSION;

	Task *task = new Task(a.arguments().at(1), a.arguments().at(2), a.arguments().at(3), &a);
	QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));
	QTimer::singleShot(0, task, SLOT(init()));

	return a.exec();
}
