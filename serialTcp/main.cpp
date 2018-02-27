/*
  Serial - TCP

	Mario Ban, Atos AG, 02.2018

*/

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QSerialPortInfo>
#include <simpleQtLogger.h>
#include "task.h"

#define TOOL_VERSION "1.0"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	Q_ASSERT_X(SQTL_VERSION >= SQTL_VERSION_CHECK(1, 2, 0), "main", "SimpleQtLogger version");

	// enable sinks
	simpleqtlogger::ENABLE_LOG_SINK_FILE = true;
	simpleqtlogger::ENABLE_LOG_SINK_CONSOLE = true;
	simpleqtlogger::ENABLE_LOG_SINK_QDEBUG = false;
	simpleqtlogger::ENABLE_LOG_SINK_SIGNAL = false;
	// set log-features
	simpleqtlogger::ENABLE_FUNCTION_STACK_TRACE = true;
	simpleqtlogger::ENABLE_CONSOLE_COLOR = false;
	simpleqtlogger::ENABLE_CONSOLE_TRIMMED = false;
	// set log-levels (global; all enabled)
	simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_FATAL = true;
	simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_ERROR = true;
	simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_WARNING = true;
	simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_NOTE = true;
	simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_INFO = true;
	simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_DEBUG = true;
	simpleqtlogger::ENABLE_LOG_LEVELS.logLevel_FUNCTION = true;
	// set log-levels (specific)
	simpleqtlogger::EnableLogLevels enableLogLevels_file = simpleqtlogger::ENABLE_LOG_LEVELS;
	simpleqtlogger::EnableLogLevels enableLogLevels_console = simpleqtlogger::ENABLE_LOG_LEVELS;
	enableLogLevels_console.logLevel_DEBUG = false;
	enableLogLevels_console.logLevel_FUNCTION = false;

	// initialize SimpleQtLogger (step 1/2)
	simpleqtlogger::SimpleQtLogger::createInstance(qApp)->setLogFormat_file("<TS> [<TID32>] [<LL>] <TEXT> (<FUNC>@<FILE>:<LINE>)", "<TS> [<TID32>] [<LL>] <TEXT>");
	simpleqtlogger::SimpleQtLogger::getInstance()->setLogFileName("serialTcp.log", 10 * 1000 * 1000, 10);
	simpleqtlogger::SimpleQtLogger::getInstance()->setLogLevels_file(enableLogLevels_file);
	simpleqtlogger::SimpleQtLogger::getInstance()->setLogLevels_console(enableLogLevels_console);

	L_INFO(QString("%1 v%2").arg(a.arguments().at(0)).arg(TOOL_VERSION));

	// TODO use QCommandLineParser

	if (argc < 4) {
		L_INFO(QString("\n"
			"ERROR: missing command-line parameter\n\n"
			"Usage: %1 serialPort localIp localPort\n"
			"  serialPort serial port this program is opening\n"
			"  localIp IP-address this program is binding to, 'any' for any interface\n"
			"  localPort port used by this program, listening\n\n"
			"Serial ports:"
		).arg(a.arguments().at(0)));
		QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();
		foreach(QSerialPortInfo serialPortInfo, serialPortInfoList) {
			L_INFO(QString("  '%1'").arg(serialPortInfo.portName()));
		}
		exit(1);
	}

	// Task *task = new Task(a.arguments().at(1), a.arguments().at(2), a.arguments().at(3), &a); // problem: children of task are deleted after SimpleQtLogger !!!
	Task task(a.arguments().at(1), a.arguments().at(2), a.arguments().at(3), &a);
	QObject::connect(&task, SIGNAL(finished()), &a, SLOT(quit()));
	QTimer::singleShot(0, &task, SLOT(init()));

	// initialize SimpleQtLogger (step 2/2)
	// simpleqtlogger::SimpleQtLogger::getInstance()->setParent(task);

	return a.exec();
}
