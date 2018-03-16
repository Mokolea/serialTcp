/*
  Serial - TCP

	Mario Ban, Atos AG, 02.2018

*/

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QCommandLineParser>
#include <QSerialPortInfo>
#include <simpleQtLogger.h>
#include "task.h"

#define TOOL_NAME "Serial-TCP"
#define TOOL_VERSION "v1.0"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(TOOL_NAME);
	QCoreApplication::setApplicationVersion(TOOL_VERSION);

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

	// L_INFO(QString("%1 %2").arg(a.arguments().at(0)).arg(TOOL_VERSION));

	// handle command-line arguments
	QCommandLineParser parser;
	parser.setApplicationDescription("Open serial port and connect to all active TCP streams.");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("serialPort", QCoreApplication::translate("main", "Serial port this program is opening."));
	parser.addPositionalArgument("serialBaud", QCoreApplication::translate("main", "Data baud rate for serial port."));
	parser.addPositionalArgument("localIp", QCoreApplication::translate("main", "IP-address this program is binding to, 'any' for any interface."));
	parser.addPositionalArgument("localPort", QCoreApplication::translate("main", "Port used by this program, listening."));
	// boolean option with multiple names (-i, --local-input)
	QCommandLineOption optionLocalInput(QStringList() << "i" << "local-input", QCoreApplication::translate("main", "Activate local input. (not implemented)"));
	parser.addOption(optionLocalInput);
	// boolean option with multiple names (-o, --local-output)
	QCommandLineOption optionLocalOutput(QStringList() << "o" << "local-output", QCoreApplication::translate("main", "Activate local output."));
	parser.addOption(optionLocalOutput);
	// process the actual command line arguments given by the user
	parser.process(a);
	const QStringList args = parser.positionalArguments();

	if (args.size() < 4) {
		// parser.showVersion();
		L_INFO(QString("\n"
			"ERROR: Missing command-line arguments\n\n"
			"Serial ports:"
		));
		QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();
		foreach(QSerialPortInfo serialPortInfo, serialPortInfoList) {
			L_INFO(QString("  '%1'").arg(serialPortInfo.portName()));
		}
		// parser.showHelp(1);
		exit(1);
	}

	// Task *task = new Task(a.arguments().at(1), a.arguments().at(2), a.arguments().at(3), a.arguments().at(4), &a); // problem: children of task are deleted after SimpleQtLogger !!!
	Task task(args.at(0), args.at(1), args.at(2), args.at(3), parser.isSet(optionLocalInput), parser.isSet(optionLocalOutput), &a);
	QObject::connect(&task, SIGNAL(finished()), &a, SLOT(quit()));
	QTimer::singleShot(0, &task, SLOT(init()));

	// initialize SimpleQtLogger (step 2/2)
	// simpleqtlogger::SimpleQtLogger::getInstance()->setParent(task);

	return a.exec();
}
