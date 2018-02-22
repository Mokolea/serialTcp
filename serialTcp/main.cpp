/*
  Serial - TCP

	Mario Ban, Atos AG, 02.2018

*/

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <simpleQtLogger.h>
#include "task.h"

#define TOOL_VERSION "1.0"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	Q_ASSERT_X(SQTL_VERSION >= SQTL_VERSION_CHECK(1, 2, 0), "main", "SimpleQtLogger version");

	// enable sinks
	simpleqtlogger::ENABLE_LOG_SINK_FILE = true;
	simpleqtlogger::ENABLE_LOG_SINK_CONSOLE = false;
	simpleqtlogger::ENABLE_LOG_SINK_QDEBUG = false;
	simpleqtlogger::ENABLE_LOG_SINK_SIGNAL = false;
	// set log-features
	simpleqtlogger::ENABLE_FUNCTION_STACK_TRACE = true;
	simpleqtlogger::ENABLE_CONSOLE_COLOR = false;
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

	// initialize SimpleQtLogger (step 1/2)
	simpleqtlogger::SimpleQtLogger::createInstance(qApp)->setLogFormat_file("<TS> [<TID32>] [<LL>] <TEXT> (<FUNC>@<FILE>:<LINE>)", "<TS> [<TID32>] [<LL>] <TEXT>");
	simpleqtlogger::SimpleQtLogger::getInstance()->setLogFileName("serialTcp.log", 10 * 1000 * 1000, 10);
	simpleqtlogger::SimpleQtLogger::getInstance()->setLogLevels_file(enableLogLevels_file);

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

	// Task *task = new Task(a.arguments().at(1), a.arguments().at(2), a.arguments().at(3), &a); // problem: children of task are deleted after SimpleQtLogger !!!
	Task task(a.arguments().at(1), a.arguments().at(2), a.arguments().at(3), &a);
	QObject::connect(&task, SIGNAL(finished()), &a, SLOT(quit()));
	QTimer::singleShot(0, &task, SLOT(init()));

	// initialize SimpleQtLogger (step 2/2)
	// simpleqtlogger::SimpleQtLogger::getInstance()->setParent(task);

	return a.exec();
}
