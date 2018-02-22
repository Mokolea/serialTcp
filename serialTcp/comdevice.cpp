/*
  Serial - TCP

  Mario Ban, Atos AG, 02.2018

*/

#include "comdevice.h"

ComDevice::ComDevice(QObject *parent)
	: QObject(parent)
{}

ComDevice::~ComDevice()
{}

// -------------------------------------------------------------------------------------------------

ComDeviceSerial::ComDeviceSerial(const QString& serialPort, QObject *parent)
	: ComDevice(parent)
	, _serialPort(serialPort)
{}

ComDeviceSerial::~ComDeviceSerial()
{}

void ComDeviceSerial::init()
{
}

// -------------------------------------------------------------------------------------------------

ComDeviceTcp::ComDeviceTcp(const QString& localIp, const QString& localPort, QObject *parent)
	: ComDevice(parent)
	, _localIp(localIp)
	, _localPort(localPort)
{}

ComDeviceTcp::~ComDeviceTcp()
{}

void ComDeviceTcp::init()
{
}
