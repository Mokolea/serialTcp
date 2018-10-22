# Serial - TCP

Open serial port and connect to all active TCP streams.

[![Release](https://img.shields.io/github/release/Mokolea/serialTcp.svg)](https://github.com/Mokolea/serialTcp/releases)
[![License](https://img.shields.io/github/license/Mokolea/serialTcp.svg)](LICENSE)

## TODO
 - Support local input

## Info
Tested on platforms:
 - Ubuntu 16.04, 18.04 using Qt 5.7
 - Raspberry Pi Model 3, Raspbian 9 (stretch) using Qt 5.7
 - Windows 10, VS 2015 using Qt 5.7

Usage:
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

*MIT License*

-- Mario
