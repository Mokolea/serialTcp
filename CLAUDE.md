# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

serialTcp is a small Qt console app (Qt 6, previously Qt 5) that opens a serial port and bridges it bidirectionally to every connected TCP client of a listening TCP server. No auth or access restrictions on incoming connections, by design.

`SimpleQtLogger/` is a git submodule (`../SimpleQtLogger.git`, relative URL) compiled directly into the app. It has its own `CLAUDE.md`; treat it as a separate repo and commit changes there, not here. `main.cpp` requires a minimum `SQTL_VERSION` (1.4.0) with a `static_assert`, so an older submodule fails the build.

## Build and run

qmake project (`serialTcp.pro`, modules `core serialport network`). Needs the Qt SerialPort module installed. `vs-project/` is an old Visual Studio project kept for Windows.

```sh
git submodule update --init      # if SimpleQtLogger/ is empty
qmake && make                    # in-source; Makefile, binary and objects are gitignored
./serialTcp -h                   # usage
./serialTcp -l                   # list serial ports
./serialTcp ttyUSB0 115200 any 65432    # then connect with: nc <host> 65432
```

There are no tests. The old Travis CI (`.travis.yml`, Qt 5) only built and ran `-h`, `-v`, `-l` as a smoke test; do the same to check a change. The app writes `serialTcp.log` (rolling, via SimpleQtLogger) in the working directory.

## Architecture

All code is in `src/`:

- `main.cpp`: configures SimpleQtLogger sinks/levels, parses the CLI (`QCommandLineParser`), constructs `Task` on the stack, and starts it via `QTimer::singleShot(0, ...)`. `Task` is deliberately a stack object, not heap-allocated with a parent: its children must be destroyed before the SimpleQtLogger instance (see comment in `main.cpp`). Version string is `TOOL_VERSION` there.
- `comdevice.h/.cpp`: `ComDevice` base `QObject` with a uniform interface: `init()` slot, `slotDataSend(QByteArray)` slot, `signalDataRecv(QByteArray)` and `finished()` signals. Three subclasses, each can be compiled out via `COMDEVICE_ENABLE_SERIAL/TCP/SCREEN` defines (default 1, overrides commented out in `serialTcp.pro`):
  - `ComDeviceSerial`: `QSerialPort`, fixed 8N1, no flow control. Any positive integer baud rate is passed to `QSerialPort`; anything else (non-numeric, zero or negative) fails `init()`.
  - `ComDeviceTcp`: `QTcpServer` plus a list of client sockets; outgoing data is broadcast to all clients. Two modes: `TEXT` (default; multiple clients, per-client byte buffer in `_dataRecv`, forwards only complete `\n`-terminated lines with a trailing `\r` removed) and `BINARY` (`-m`; raw pass-through, and only one client is accepted at a time). A failing or slow client, or one exceeding the line limit, is dropped via `dropClient()` without stopping the server; limits are the constants at the top of `comdevice.cpp`.
  - `ComDeviceScreen`: local stdout output (`-o`), raw bytes via `QFile`. `main.cpp` turns off the SimpleQtLogger console sink with `-o`, because it also writes to stdout. Local input (`-i`) is not implemented (only a warning is logged).
- `task.cpp`: `Task::init()` creates the three devices, wires `signalDataRecv` of one device to `slotDataSend` of another (serial↔TCP always; serial→screen if `-o`; screen→serial if `-i`), and connects every device's `finished()` to `Task::finished(int exitCode)`, which `main.cpp` connects to `QCoreApplication::exit`. Devices only emit `finished()` on failure, so all route through `Task::slotFailed()` and exit with code 1. Any device finishing terminates the whole program. Exit codes are documented in `README.md`; keep them in sync.

To add a new endpoint type, subclass `ComDevice` and wire it in `Task::init()`.

## Conventions

- Logging uses SimpleQtLogger macros (`L_FUNC`, `L_INFO`, `L_NOTE`, `L_ERROR`, ...), not `qDebug`.
- 2-space indentation in `main.cpp`/`task.cpp`; `comdevice.*` indents class bodies by 4 with access specifiers at 2. Match the surrounding file.
- Each source file starts with the MIT license header block.
- Keep the usage text in `README.md` in sync with the CLI options in `main.cpp`.
