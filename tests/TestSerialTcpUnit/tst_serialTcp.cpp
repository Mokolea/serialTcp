/*

Serial - TCP

Mario Ban, 2018-02
https://github.com/Mokolea/serialTcp


MIT License

Copyright (c) 2026 Mario Ban

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

// No SimpleQtLogger instance is created: the L_* macros and L_FUNC are no-ops then,
// so the tests write no log file. No serial hardware is needed.

#include <QtTest>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryFile>
#include <cstdio>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include "comdevice.h"
#include "task.h"

namespace {
// keep in sync with the limits at the top of src/comdevice.cpp
const int TCP_LINE_SIZE_MAX = 64 * 1024;
const qint64 TCP_SEND_BUFFER_SIZE_MAX = 4 * 1024 * 1024;

const QString SERIAL_PORT_NONEXISTENT = "serialTcp-nonexistent";
}

class TestSerialTcp : public QObject
{
  Q_OBJECT

private slots:
  void init();
  void cleanup();

  // ComDeviceSerial
  void serialBaudRateInvalid_data();
  void serialBaudRateInvalid();
  void serialPortOpenFails();
  void serialSendBeforeInit();

  // ComDeviceTcp init
  void tcpAddressInvalid();
  void tcpPortInvalid_data();
  void tcpPortInvalid();
  void tcpListenFails();
  void tcpAnyAddress_data();
  void tcpAnyAddress();
  void tcpSendBeforeInit();

  // ComDeviceTcp Mode::TEXT, client -> device
  void textCompleteLine();
  void textCrlf();
  void textMultipleLinesOneWrite();
  void textPartialAcrossWrites();
  void textUtf8Split();
  void textClientsDoNotInterleave();
  void textLineTooLong();
  void textLineAtLimitKept();

  // ComDeviceTcp device -> clients
  void sendBroadcast();
  void sendAfterClientDisconnect();
  void sendSlowClientDropped();

  // ComDeviceTcp Mode::BINARY
  void binaryPassThrough();
  void binarySecondClientRejected();

  // ComDeviceScreen
  void screenWritesRawBytes();
  void screenSendBeforeInit();

  // Task
  void taskSerialBaudRateInvalidExits();
  void taskSerialOpenFailsExits();

private:
  ComDeviceTcp* startTcp(ComDeviceTcp::Mode mode);
  QTcpServer* server(ComDeviceTcp* dev);
  QTcpSocket* connectClient(ComDeviceTcp* dev, int acceptedExpected);
  static QByteArray joined(const QSignalSpy& spy);
  static bool waitBytes(QTcpSocket* client, qint64 size);

  QObject* _scope; // parent of all devices and clients of one test, deleted in cleanup()
};

// -------------------------------------------------------------------------------------------------

void TestSerialTcp::init()
{
  _scope = new QObject;
}

void TestSerialTcp::cleanup()
{
  delete _scope;
  _scope = 0;
}

ComDeviceTcp* TestSerialTcp::startTcp(ComDeviceTcp::Mode mode)
{
  // port 0: any free port, read back from the device's QTcpServer
  ComDeviceTcp* dev = new ComDeviceTcp("127.0.0.1", "0", mode, _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->init();
  if (spyFinished.count() != 0 || !server(dev) || !server(dev)->isListening()) {
    return 0;
  }
  return dev;
}

QTcpServer* TestSerialTcp::server(ComDeviceTcp* dev)
{
  return dev->findChild<QTcpServer*>();
}

QTcpSocket* TestSerialTcp::connectClient(ComDeviceTcp* dev, int acceptedExpected)
{
  QTcpSocket* client = new QTcpSocket(_scope);
  client->connectToHost(QHostAddress::LocalHost, server(dev)->serverPort());
  if (!client->waitForConnected(5000)) {
    return 0;
  }
  // accepted sockets are children of the QTcpServer; wait until the device took this one
  if (!QTest::qWaitFor([&]() { return server(dev)->findChildren<QTcpSocket*>().size() == acceptedExpected; }, 5000)) {
    return 0;
  }
  return client;
}

QByteArray TestSerialTcp::joined(const QSignalSpy& spy)
{
  QByteArray data;
  for (const QList<QVariant>& args : spy) {
    data += args.at(0).toByteArray();
  }
  return data;
}

bool TestSerialTcp::waitBytes(QTcpSocket* client, qint64 size)
{
  return QTest::qWaitFor([&]() { return client->bytesAvailable() >= size; }, 5000);
}

// -------------------------------------------------------------------------------------------------

void TestSerialTcp::serialBaudRateInvalid_data()
{
  QTest::addColumn<QString>("baudRate");
  QTest::newRow("empty") << "";
  QTest::newRow("text") << "abc";
  QTest::newRow("zero") << "0";
  QTest::newRow("negative") << "-9600";
  QTest::newRow("trailing") << "9600x";
}

void TestSerialTcp::serialBaudRateInvalid()
{
  QFETCH(QString, baudRate);
  ComDeviceSerial* dev = new ComDeviceSerial(SERIAL_PORT_NONEXISTENT, baudRate, _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->init();
  QCOMPARE(spyFinished.count(), 1);
}

void TestSerialTcp::serialPortOpenFails()
{
  ComDeviceSerial* dev = new ComDeviceSerial(SERIAL_PORT_NONEXISTENT, "115200", _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->init();
  QCOMPARE(spyFinished.count(), 1);
  QTest::qWait(50); // no second report via errorOccurred
  QCOMPARE(spyFinished.count(), 1);
}

void TestSerialTcp::serialSendBeforeInit()
{
  ComDeviceSerial* dev = new ComDeviceSerial(SERIAL_PORT_NONEXISTENT, "115200", _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->slotDataSend("data");
  QCOMPARE(spyFinished.count(), 0);
}

// -------------------------------------------------------------------------------------------------

void TestSerialTcp::tcpAddressInvalid()
{
  ComDeviceTcp* dev = new ComDeviceTcp("not-an-ip", "0", ComDeviceTcp::Mode::TEXT, _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->init();
  QCOMPARE(spyFinished.count(), 1);
}

void TestSerialTcp::tcpPortInvalid_data()
{
  QTest::addColumn<QString>("port");
  QTest::newRow("text") << "abc";
  QTest::newRow("too-large") << "70000";
  QTest::newRow("negative") << "-1";
}

void TestSerialTcp::tcpPortInvalid()
{
  QFETCH(QString, port);
  ComDeviceTcp* dev = new ComDeviceTcp("127.0.0.1", port, ComDeviceTcp::Mode::TEXT, _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->init();
  QCOMPARE(spyFinished.count(), 1);
}

void TestSerialTcp::tcpListenFails()
{
  QTcpServer occupied;
  QVERIFY(occupied.listen(QHostAddress::LocalHost, 0));
  ComDeviceTcp* dev = new ComDeviceTcp("127.0.0.1", QString::number(occupied.serverPort()), ComDeviceTcp::Mode::TEXT, _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->init();
  QCOMPARE(spyFinished.count(), 1);
}

void TestSerialTcp::tcpAnyAddress_data()
{
  QTest::addColumn<QString>("address");
  QTest::newRow("lower") << "any";
  QTest::newRow("upper") << "ANY";
}

void TestSerialTcp::tcpAnyAddress()
{
  QFETCH(QString, address);
  ComDeviceTcp* dev = new ComDeviceTcp(address, "0", ComDeviceTcp::Mode::TEXT, _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->init();
  QCOMPARE(spyFinished.count(), 0);
  QVERIFY(server(dev));
  QVERIFY(server(dev)->isListening());
  QCOMPARE(server(dev)->serverAddress(), QHostAddress(QHostAddress::Any));
}

void TestSerialTcp::tcpSendBeforeInit()
{
  ComDeviceTcp* dev = new ComDeviceTcp("127.0.0.1", "0", ComDeviceTcp::Mode::TEXT, _scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->slotDataSend("data");
  QCOMPARE(spyFinished.count(), 0);
}

// -------------------------------------------------------------------------------------------------

void TestSerialTcp::textCompleteLine()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* client = connectClient(dev, 1);
  QVERIFY(client);

  client->write("hello\n");
  QTRY_COMPARE(spyRecv.count(), 1);
  QCOMPARE(spyRecv.at(0).at(0).toByteArray(), QByteArray("hello\n"));
}

void TestSerialTcp::textCrlf()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* client = connectClient(dev, 1);
  QVERIFY(client);

  client->write("a\r\nb\rc\r\n");
  QTRY_COMPARE(spyRecv.count(), 2);
  QCOMPARE(spyRecv.at(0).at(0).toByteArray(), QByteArray("a\n"));
  QCOMPARE(spyRecv.at(1).at(0).toByteArray(), QByteArray("b\rc\n")); // only a trailing CR is removed
}

void TestSerialTcp::textMultipleLinesOneWrite()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* client = connectClient(dev, 1);
  QVERIFY(client);

  client->write("a\nb\nc");
  QTRY_COMPARE(spyRecv.count(), 2);
  QCOMPARE(spyRecv.at(0).at(0).toByteArray(), QByteArray("a\n"));
  QCOMPARE(spyRecv.at(1).at(0).toByteArray(), QByteArray("b\n"));

  client->write("\n");
  QTRY_COMPARE(spyRecv.count(), 3);
  QCOMPARE(spyRecv.at(2).at(0).toByteArray(), QByteArray("c\n"));
}

void TestSerialTcp::textPartialAcrossWrites()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* client = connectClient(dev, 1);
  QVERIFY(client);

  client->write("hel");
  QVERIFY(client->waitForBytesWritten(5000));
  QTest::qWait(50);
  QCOMPARE(spyRecv.count(), 0); // incomplete line is held back

  client->write("lo\n");
  QTRY_COMPARE(spyRecv.count(), 1);
  QCOMPARE(spyRecv.at(0).at(0).toByteArray(), QByteArray("hello\n"));
}

void TestSerialTcp::textUtf8Split()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* client = connectClient(dev, 1);
  QVERIFY(client);

  const QByteArray utf8 = QString::fromUtf8("\xc3\xa4").toUtf8(); // a-umlaut, 2 bytes
  QCOMPARE(utf8.size(), 2);
  client->write(utf8.left(1));
  QVERIFY(client->waitForBytesWritten(5000));
  QTest::qWait(50);
  client->write(utf8.mid(1) + "\n");
  QTRY_COMPARE(spyRecv.count(), 1);
  QCOMPARE(spyRecv.at(0).at(0).toByteArray(), utf8 + "\n");
}

void TestSerialTcp::textClientsDoNotInterleave()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* clientA = connectClient(dev, 1);
  QVERIFY(clientA);
  QTcpSocket* clientB = connectClient(dev, 2);
  QVERIFY(clientB);

  clientA->write("aa");
  QVERIFY(clientA->waitForBytesWritten(5000));
  QTest::qWait(50);
  clientB->write("bb\n");
  QTRY_COMPARE(spyRecv.count(), 1);
  QCOMPARE(spyRecv.at(0).at(0).toByteArray(), QByteArray("bb\n"));

  clientA->write("a\n");
  QTRY_COMPARE(spyRecv.count(), 2);
  QCOMPARE(spyRecv.at(1).at(0).toByteArray(), QByteArray("aaa\n"));
}

void TestSerialTcp::textLineTooLong()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* clientLong = connectClient(dev, 1);
  QVERIFY(clientLong);
  QTcpSocket* clientOther = connectClient(dev, 2);
  QVERIFY(clientOther);

  clientLong->write(QByteArray(TCP_LINE_SIZE_MAX + 1, 'x'));
  QTRY_COMPARE(clientLong->state(), QAbstractSocket::UnconnectedState); // dropped
  QCOMPARE(spyRecv.count(), 0);

  // server keeps running for the others
  clientOther->write("still here\n");
  QTRY_COMPARE(spyRecv.count(), 1);
  QCOMPARE(spyRecv.at(0).at(0).toByteArray(), QByteArray("still here\n"));
  QCOMPARE(spyFinished.count(), 0);
}

void TestSerialTcp::textLineAtLimitKept()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* client = connectClient(dev, 1);
  QVERIFY(client);

  const QByteArray line(TCP_LINE_SIZE_MAX, 'x');
  client->write(line); // exactly at the limit, buffered without LF
  QVERIFY(client->waitForBytesWritten(5000));
  QTest::qWait(200);
  QCOMPARE(client->state(), QAbstractSocket::ConnectedState);

  client->write("\n");
  QTRY_COMPARE(spyRecv.count(), 1);
  QCOMPARE(spyRecv.at(0).at(0).toByteArray(), line + "\n");
  QCOMPARE(client->state(), QAbstractSocket::ConnectedState);
}

// -------------------------------------------------------------------------------------------------

void TestSerialTcp::sendBroadcast()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QTcpSocket* clientA = connectClient(dev, 1);
  QVERIFY(clientA);
  QTcpSocket* clientB = connectClient(dev, 2);
  QVERIFY(clientB);

  dev->slotDataSend("x\r\ny"); // device -> clients is sent unchanged, also in Mode::TEXT
  QVERIFY(waitBytes(clientA, 4));
  QVERIFY(waitBytes(clientB, 4));
  QCOMPARE(clientA->readAll(), QByteArray("x\r\ny"));
  QCOMPARE(clientB->readAll(), QByteArray("x\r\ny"));
}

void TestSerialTcp::sendAfterClientDisconnect()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  QTcpSocket* clientA = connectClient(dev, 1);
  QVERIFY(clientA);
  QTcpSocket* clientB = connectClient(dev, 2);
  QVERIFY(clientB);

  clientA->disconnectFromHost();
  QTRY_COMPARE(server(dev)->findChildren<QTcpSocket*>().size(), 1); // server side cleaned up

  dev->slotDataSend("after");
  QVERIFY(waitBytes(clientB, 5));
  QCOMPARE(clientB->readAll(), QByteArray("after"));
  QCOMPARE(spyFinished.count(), 0);
}

void TestSerialTcp::sendSlowClientDropped()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::TEXT);
  QVERIFY(dev);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  QTcpSocket* client = connectClient(dev, 1);
  QVERIFY(client);

  // QTcpSocket buffers a write, so bytesToWrite() is above the limit right after a large send
  dev->slotDataSend(QByteArray(TCP_SEND_BUFFER_SIZE_MAX + 1, 'x'));
  QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
  QTRY_COMPARE(server(dev)->findChildren<QTcpSocket*>().size(), 0);
  QCOMPARE(spyFinished.count(), 0);

  // a new client is served normally
  QTcpSocket* clientNew = connectClient(dev, 1);
  QVERIFY(clientNew);
  dev->slotDataSend("ok");
  QVERIFY(waitBytes(clientNew, 2));
  QCOMPARE(clientNew->readAll(), QByteArray("ok"));
}

// -------------------------------------------------------------------------------------------------

void TestSerialTcp::binaryPassThrough()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::BINARY);
  QVERIFY(dev);
  QSignalSpy spyRecv(dev, &ComDevice::signalDataRecv);
  QTcpSocket* client = connectClient(dev, 1);
  QVERIFY(client);

  const QByteArray data("a\r\nb\0\xff", 6);
  client->write(data);
  QTRY_COMPARE(joined(spyRecv), data); // no line handling, may arrive in several parts
}

void TestSerialTcp::binarySecondClientRejected()
{
  ComDeviceTcp* dev = startTcp(ComDeviceTcp::Mode::BINARY);
  QVERIFY(dev);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  QTcpSocket* clientA = connectClient(dev, 1);
  QVERIFY(clientA);

  QTcpSocket* clientB = new QTcpSocket(_scope);
  clientB->connectToHost(QHostAddress::LocalHost, server(dev)->serverPort());
  QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState); // rejected
  QTRY_COMPARE(server(dev)->findChildren<QTcpSocket*>().size(), 1);

  dev->slotDataSend("A");
  QVERIFY(waitBytes(clientA, 1));
  QCOMPARE(clientA->readAll(), QByteArray("A"));

  // after the first client is gone, a new one is accepted
  clientA->disconnectFromHost();
  QTRY_COMPARE(server(dev)->findChildren<QTcpSocket*>().size(), 0);
  QTcpSocket* clientC = connectClient(dev, 1);
  QVERIFY(clientC);
  dev->slotDataSend("C");
  QVERIFY(waitBytes(clientC, 1));
  QCOMPARE(clientC->readAll(), QByteArray("C"));
  QCOMPARE(spyFinished.count(), 0);
}

// -------------------------------------------------------------------------------------------------

void TestSerialTcp::screenWritesRawBytes()
{
#ifdef Q_OS_UNIX
  QTemporaryFile file;
  QVERIFY(file.open());

  // redirect stdout to the file while the device writes; no QTest output in between
  std::fflush(stdout);
  const int stdoutSaved = ::dup(STDOUT_FILENO);
  QVERIFY(stdoutSaved >= 0);
  const bool redirected = ::dup2(file.handle(), STDOUT_FILENO) >= 0;

  int finishedCount = -1;
  const QByteArray data("raw\0\xff\n", 6);
  if (redirected) {
    ComDeviceScreen* dev = new ComDeviceScreen(_scope);
    QSignalSpy spyFinished(dev, &ComDevice::finished);
    dev->init();
    dev->slotDataSend(data);
    finishedCount = spyFinished.count();
    std::fflush(stdout);
  }

  ::dup2(stdoutSaved, STDOUT_FILENO);
  ::close(stdoutSaved);

  QVERIFY(redirected);
  QCOMPARE(finishedCount, 0);
  file.seek(0);
  QCOMPARE(file.readAll(), data);
#else
  QSKIP("stdout redirection needs POSIX dup2()");
#endif
}

void TestSerialTcp::screenSendBeforeInit()
{
  ComDeviceScreen* dev = new ComDeviceScreen(_scope);
  QSignalSpy spyFinished(dev, &ComDevice::finished);
  dev->slotDataSend("data");
  QCOMPARE(spyFinished.count(), 0);
}

// -------------------------------------------------------------------------------------------------

void TestSerialTcp::taskSerialBaudRateInvalidExits()
{
  Task* task = new Task(SERIAL_PORT_NONEXISTENT, "abc", "127.0.0.1", "0", ComDeviceTcp::Mode::TEXT, false, false, _scope);
  QSignalSpy spyFinished(task, &Task::finished);
  task->init(); // devices are initialized from the event loop
  QTRY_COMPARE(spyFinished.count(), 1);
  QCOMPARE(spyFinished.at(0).at(0).toInt(), 1);
}

void TestSerialTcp::taskSerialOpenFailsExits()
{
  Task* task = new Task(SERIAL_PORT_NONEXISTENT, "115200", "127.0.0.1", "0", ComDeviceTcp::Mode::TEXT, false, false, _scope);
  QSignalSpy spyFinished(task, &Task::finished);
  task->init();
  QTRY_COMPARE(spyFinished.count(), 1);
  QCOMPARE(spyFinished.at(0).at(0).toInt(), 1);
}

QTEST_GUILESS_MAIN(TestSerialTcp)

#include "tst_serialTcp.moc"
