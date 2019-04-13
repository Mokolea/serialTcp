# Serial - TCP [![Build Status](https://travis-ci.org/Mokolea/serialTcp.svg)](https://travis-ci.org/Mokolea/serialTcp)

Provide remote (network) access to a serial port. Built using the [Qt Framework](https://doc.qt.io).

[![Release](https://img.shields.io/github/release/Mokolea/serialTcp.svg)](https://github.com/Mokolea/serialTcp/releases)
[![License](https://img.shields.io/github/license/Mokolea/serialTcp.svg)](LICENSE)

## Features
 - Opens a serial port and connects bidirectional all active TCP streams
 - Accepts (no restrictions, no security considerations/precautions neither) incoming TCP connections

## TODO
 - Support local input
 - Current limitations:
    - Supported baud rates: 4800, 9600, 19200, 38400, 57600, 115200
    - Immutable serial port parameters: 8N1 (8 data bits, no parity, one stop bit), no flow control

## Info
Tested on platforms:
 - Ubuntu 16.04, 18.04 using Qt 5.5.1 / 5.7
 - Raspberry Pi Model 3, Raspbian 9 (stretch) using Qt 5.7
 - macOS Sierra (10.12.6), using Qt 5.10
 - Windows 10, VS 2015 using Qt 5.7

## Usage
```
$ ./serialTcp --help
Usage: ./serialTcp [options] serialPort serialBaud localIp localPort
Open serial port and connect to all active TCP streams.
(https://github.com/Mokolea/serialTcp)

Options:
  -h, --help               Displays this help.
  -v, --version            Displays version information.
  -l, --list-serial-ports  List all currently available serial ports.
  -i, --local-input        Activate local input. (not implemented)
  -o, --local-output       Activate local output.

Arguments:
  serialPort               Serial port this program is opening.
  serialBaud               Data baud rate for serial port.
  localIp                  IP address this program is binding to, 'any' for any
                           interface.
  localPort                TCP port used by this program, listening.
$ 
```

List all currently available serial ports:
```
$ ./serialTcp -l

Serial ports:
  'ttyUSB0'
  'ttyS0'
$ 
```

## Example (using Linux)
Scenario 1: remote access logging information from a device (like Arduino, ...) attached to a serial port of Host A:
 - Host A with serial port `/dev/ttyS0` (or `/dev/ttyUSB0`, ...) and IP address `192.168.1.100`
 - Host B connecting to serial port of host A
 - Host C also connecting to serial port of host A

Host A: Prepare local serial port to be connected to from remote machines:
```
$ ./serialTcp ttyS0 115200 any 65432
```
Host B and C: Connect to serial port of host A:
```
$ nc 192.168.1.100 65432
```

## Hint: Linux socat
On Linux you can also use `socat` to access a serial port over TCP.

Example (see above): Host A:
```
$ socat TCP-LISTEN:65432,fork,reuseaddr FILE:/dev/ttyS0,115200,raw
```

*MIT License*

-- Mario
