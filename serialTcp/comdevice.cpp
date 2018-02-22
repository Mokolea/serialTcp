/*
  Serial - TCP

  Mario Ban, Atos AG, 02.2018

*/

#include "comdevice.h"
#include <simpleQtLogger.h>

ComDevice::ComDevice(QObject *parent)
	: QObject(parent)
{
	L_FUNC("");
}

ComDevice::~ComDevice()
{
	L_FUNC("");
}

// -------------------------------------------------------------------------------------------------

ComDeviceSerial::ComDeviceSerial(const QString& serialPort, QObject *parent)
	: ComDevice(parent)
	, _serialPort(serialPort)
{
	L_FUNC("");
}

ComDeviceSerial::~ComDeviceSerial()
{
	L_FUNC("");
}

void ComDeviceSerial::init()
{
	L_FUNC("");
}

// -------------------------------------------------------------------------------------------------

ComDeviceTcp::ComDeviceTcp(const QString& localIp, const QString& localPort, QObject *parent)
	: ComDevice(parent)
	, _localIp(localIp)
	, _localPort(localPort)
{
	L_FUNC("");
}

ComDeviceTcp::~ComDeviceTcp()
{
	L_FUNC("");
}

void ComDeviceTcp::init()
{
	L_FUNC("");
}
