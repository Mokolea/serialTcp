/*

Serial - TCP

Mario Ban, 2018-02
https://github.com/Mokolea/serialTcp


MIT License

Copyright (c) 2018 Mario Ban

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QCommandLineParser>
#include <QSerialPortInfo>
#include <simpleQtLogger.h>
#include "task.h"

#define TOOL_NAME "Serial-TCP"
#define TOOL_VERSION "v1.1"

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  QCoreApplication::setApplicationName(TOOL_NAME);
  QCoreApplication::setApplicationVersion(TOOL_VERSION);

  Q_ASSERT_X(SQTL_VERSION >= SQTL_VERSION_CHECK(1, 3, 0), "main", "SimpleQtLogger version");

  // enable sinks
  simpleqtlogger::ENABLE_LOG_SINK_FILE = true;
  simpleqtlogger::ENABLE_LOG_SINK_CONSOLE = true;
  simpleqtlogger::ENABLE_LOG_SINK_QDEBUG = false;
  simpleqtlogger::ENABLE_LOG_SINK_SIGNAL = false;
  simpleqtlogger::ENABLE_LOG_SINK_SYSLOG = false;
  // set log-features
  simpleqtlogger::ENABLE_FUNCTION_STACK_TRACE = true;
  simpleqtlogger::ENABLE_CONSOLE_COLOR = false;
  simpleqtlogger::ENABLE_CONSOLE_TRIMMED = false;
  simpleqtlogger::ENABLE_CONSOLE_LOG_FILE_STATE = false;
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
  simpleqtlogger::SimpleQtLogger::getInstance()->setLogLevels_file(enableLogLevels_file);
  simpleqtlogger::SimpleQtLogger::getInstance()->setLogFileName("serialTcp.log", 10 * 1000 * 1000, 10);
  simpleqtlogger::SimpleQtLogger::getInstance()->setLogLevels_console(enableLogLevels_console);

  // L_INFO(QString("%1 %2").arg(a.arguments().at(0)).arg(TOOL_VERSION));

  // handle command-line arguments
  QCommandLineParser parser;
  parser.setApplicationDescription("Open serial port and connect to all active TCP streams.\n(https://github.com/Mokolea/serialTcp)");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("serial-port", QCoreApplication::translate("main", "Serial port this program is opening."));
  parser.addPositionalArgument("serial-baud", QCoreApplication::translate("main", "Data baud rate for serial port, e.g. 115200 (8N1, no flow control)."));
  parser.addPositionalArgument("local-ip", QCoreApplication::translate("main", "IP address this program is binding to, 'any' for any interface."));
  parser.addPositionalArgument("local-port", QCoreApplication::translate("main", "TCP port used by this program, listening."));
  // boolean option with multiple names (-l, --list-serial-ports)
  QCommandLineOption optionListSerialPorts(QStringList() << "l" << "list-serial-ports", QCoreApplication::translate("main", "List all currently available serial ports."));
  parser.addOption(optionListSerialPorts);
  // boolean option with multiple names (-i, --local-input)
  QCommandLineOption optionLocalInput(QStringList() << "i" << "local-input", QCoreApplication::translate("main", "Activate local input. (not implemented)"));
  parser.addOption(optionLocalInput);
  // boolean option with multiple names (-o, --local-output)
  QCommandLineOption optionLocalOutput(QStringList() << "o" << "local-output", QCoreApplication::translate("main", "Activate local output."));
  parser.addOption(optionLocalOutput);
  // boolean option with multiple names (-m, --mode-binary)
  QCommandLineOption optionModeBinary(QStringList() << "m" << "mode-binary", QCoreApplication::translate("main", "TCP mode binary, default text (multi connection, wait for <LF>)."));
  parser.addOption(optionModeBinary);
  // process the actual command line arguments given by the user (also handles the builtin options "-h" and "-v" and handles errors, then exit)
  parser.process(a);

  const QStringList args = parser.positionalArguments();

  if (parser.isSet(optionListSerialPorts)) {
    // parser.showVersion();
    L_INFO("Serial ports:");
    QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();
    foreach(QSerialPortInfo serialPortInfo, serialPortInfoList) {
      L_INFO(QString("  '%1'").arg(serialPortInfo.portName()));
    }
    // parser.showHelp(1);
    exit(0);
  }
  else if (args.size() < 4) {
    L_ERROR("Missing command-line arguments (option -h displays help)");
    exit(1);
  }

  // Task *task = new Task(...); // problem: children of task are deleted after SimpleQtLogger !!!
  Task task(args.at(0), args.at(1), args.at(2), args.at(3), parser.isSet(optionModeBinary) ? ComDeviceTcp::Mode::BINARY : ComDeviceTcp::Mode::TEXT,
            parser.isSet(optionLocalInput), parser.isSet(optionLocalOutput), &a);
  QObject::connect(&task, SIGNAL(finished()), &a, SLOT(quit()));
  QTimer::singleShot(0, &task, SLOT(init()));

  // initialize SimpleQtLogger (step 2/2)
  // simpleqtlogger::SimpleQtLogger::getInstance()->setParent(task);

  return a.exec();
}
