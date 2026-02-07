# CLAUDE.md — AI Assistant Guide for IRMC

## Project Overview

IRMC (Internet Relay Morse Code) is a C client implementing the CWCom/MOIP (Morse Code Over IP) protocol. It connects to MorseKOB relay servers, allowing morse code operators to communicate over the internet. This is experimental code.

- **Language**: C (~818 lines across 4 modules)
- **Version**: 0.3.3
- **Platforms**: Linux, macOS, Raspberry Pi
- **Network**: UDP-based CWCom protocol (default server: mtc-kob.dyndns.org, port 7890)

## Repository Structure

```
irmc/
├── CLAUDE.md              # This file
├── README.md              # User-facing documentation
├── AUTHORS                # Contributors
├── doc/                   # Protocol and hardware PDFs
│   ├── cwcom.pdf          # CWCom protocol specification
│   └── loopinterface.pdf  # Hardware loop interface guide
├── lib/
│   └── morse-0.1.tar.gz   # Morse code utility library
└── src/
    ├── Makefile            # Build configuration
    ├── irmc.c              # Main client (entry point, networking, event loop)
    ├── cwprotocol.c/.h     # CWCom protocol packet handling
    ├── beep.c/.h           # Audio output via PortAudio (sine wave tone generation)
    ├── util.c/.h           # Cross-platform utilities (timing, keyboard, sockets)
    └── test/
        └── MorseKOB.jar    # Java test server for local development
```

## Build Commands

All build commands run from `src/`:

```bash
cd src
make              # Build for Linux/macOS
make raspi        # Build for Raspberry Pi (with wiringPi/GPIO)
make clean        # Remove build artifacts
make install      # Install binary to ~/bin
make java         # Run local MorseKOB test server
```

**Dependencies**: libportaudio, pthread, libm. Raspberry Pi builds also need wiringPi.

Install on Linux:
```bash
apt-get install build-essential libportaudio-dev portaudio19-dev
```

## Running

```bash
./irmc                                    # Defaults: mtc-kob.dyndns.org:7890 channel 103
./irmc -h morsecode.dc3.com -c 205       # Custom server and channel
./irmc -h host -p port -c chan -i callsign -s /dev/ttyUSB0
```

## Testing

There is no automated test suite. Testing is manual:
- Use `make java` to start a local MorseKOB test server
- Connect with `./irmc -h localhost -c <channel>`

## Architecture

**Entry point**: `main()` in `irmc.c`

**Main loop** (irmc.c): An infinite loop that:
1. Receives UDP packets from the server
2. Decodes and displays morse code (with audio playback)
3. Captures local morse key input (keyboard, serial, or GPIO)
4. Transmits morse timing data as CWCom packets
5. Sends keep-alive packets periodically

**Module responsibilities**:
- `irmc.c` — CLI parsing, socket setup, main event loop, signal handling
- `cwprotocol.c/h` — Packet construction (connect, disconnect, data, ID), struct definitions
- `beep.c/h` — PortAudio sine wave generation at configurable frequency
- `util.c/h` — `fastclock()` (ms timer), `kbhit()` (non-blocking keyboard), socket helpers

**Conditional compilation flags**:
- `RASPI` — Enables Raspberry Pi GPIO input (wiringPi)
- `TX_SERIAL` — Enables serial port hardware interface (currently disabled)
- `DEBUG` — Enables debug output
- `__MACH__` — macOS-specific clock implementation

## Code Conventions

- **Functions/variables**: `snake_case` (e.g., `fastclock()`, `tx_timer`)
- **Constants/macros**: `UPPER_SNAKE_CASE` (e.g., `MAXDATASIZE`, `TX_TIMEOUT`)
- **Structs**: `snake_case` with `_format` suffix (e.g., `data_packet_format`)
- **Indentation**: Tabs
- **Brace style**: K&R (opening brace on same line)
- **Comments**: Mix of `//` and `/* */`
- **State management**: Global variables for networking and protocol state
- **Platform differences**: Handled via `#ifdef` preprocessor blocks

## Protocol Details

- **Transport**: UDP (SOCK_DGRAM)
- **Data packet size**: 496 bytes (command, ID, sequence number, morse timing codes, status)
- **Command types**: CON (0x0004), DIS (0x0002), DAT (0x0003), ACK (0x0005)
- **Morse encoding**: Array of signed ints — positive values = tone/mark duration (ms), negative = space duration
- **Keep-alive**: Sent every ~5 seconds
- **TX timeout**: 240 seconds of inactivity

## Key Constants (cwprotocol.h)

- `SIZE_DATA_PACKET` = 496
- `SIZE_CODE` = 51 (max morse timing elements per packet)
- `INTERFACEVERSION` = "irmc v0.3.3"
- Default channel: 103, default port: 7890

## Commit Style

This repository uses very short, informal commit messages (e.g., `wip`, `url`, `raspi`, `code indent`). Match this style for consistency.
