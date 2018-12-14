# Serial - TCP [![Build Status](https://travis-ci.org/Mokolea/serialTcp.svg)](https://travis-ci.org/Mokolea/serialTcp)

Provide remote (network) access to a serial port. Built using the [Qt Framework](https://doc.qt.io).

[![Release](https://img.shields.io/github/release/Mokolea/serialTcp.svg)](https://github.com/Mokolea/serialTcp/releases)
[![License](https://img.shields.io/github/license/Mokolea/serialTcp.svg)](LICENSE)

## Features
 - Opens a serial port and connects bidirectional all active TCP streams
 - Accepts (no restrictions, no security considerations/precautions neither) incoming TCP connections

## TODO
 - Support local input

## Info
Tested on platforms:
 - Ubuntu 16.04, 18.04 using Qt 5.7
 - Raspberry Pi Model 3, Raspbian 9 (stretch) using Qt 5.7
 - Windows 10, VS 2015 using Qt 5.7

## Usage
```
$ ./serialTcp --help
Current log-file: "serialTcp.log" role "main"
Usage: ./serialTcp [options] serialPort serialBaud localIp localPort
Open serial port and connect to all active TCP streams.

Options:
  -h, --help          Displays this help.
  -v, --version       Displays version information.
  -i, --local-input   Activate local input. (not implemented)
  -o, --local-output  Activate local output.

Arguments:
  serialPort          Serial port this program is opening.
  serialBaud          Data baud rate for serial port.
  localIp             IP-address this program is binding to, 'any' for any
                      interface.
  localPort           Port used by this program, listening.
$ 
```

```
$ ./serialTcp 
Current log-file: "serialTcp.log" role "main"

ERROR: Missing command-line arguments

Serial ports:
  'ttyAMA0'
  'ttyS0'
$ 
```

*MIT License*

-- Mario
