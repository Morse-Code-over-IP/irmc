# IRMC — Internet Relay Morse Code

**A lightweight C client for the CWCom / Morse-over-IP (MOIP) protocol.**

IRMC connects to [MorseKOB](http://kob.sdf.org/) relay servers and lets morse code operators communicate over the internet — just like IRC, but in morse code. Plug in a telegraph key (via serial or GPIO), or simply listen to traffic on a channel.

> **Status:** Experimental — works on Linux, macOS, and Raspberry Pi.

---

## Table of Contents

- [Quick Start](#quick-start)
- [Building](#building)
- [Usage](#usage)
- [The CWCom Protocol](#the-cwcom-protocol)
- [Architecture](#architecture)
- [Hardware Interfaces](#hardware-interfaces)
- [Testing](#testing)
- [Changelog](#changelog)
- [Authors](#authors)

---

## Quick Start

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt-get install -y build-essential libportaudio-dev portaudio19-dev

# Build
cd src && make

# Connect to a channel and listen
./irmc -h morsecode.dc3.com -c 205
```

---

## Building

### Linux (Debian / Ubuntu)

```bash
sudo apt-get install -y build-essential libportaudio-dev portaudio19-dev
cd src
make
```

### Raspberry Pi (with GPIO key input)

```bash
sudo apt-get install -y build-essential libportaudio-dev portaudio19-dev wiringpi
cd src
make raspi
```

### macOS

```bash
brew install portaudio
cd src
make
```

### Other Make Targets

| Target         | Description                              |
|:---------------|:-----------------------------------------|
| `make`         | Build `irmc` binary (Linux / macOS)      |
| `make raspi`   | Build with Raspberry Pi GPIO support     |
| `make clean`   | Remove build artifacts (`*.o`, binary)   |
| `make install` | Copy binary to `~/bin`                   |
| `make java`    | Start local MorseKOB test server (Java)  |

**Dependencies:** libportaudio, pthread, libm. Raspberry Pi builds additionally require wiringPi.

---

## Usage

```
irmc [options]
```

| Option | Description                        | Default                  |
|:-------|:-----------------------------------|:-------------------------|
| `-h`   | Server hostname                    | `mtc-kob.dyndns.org`    |
| `-p`   | Server port                        | `7890`                   |
| `-c`   | Channel number                     | `103`                    |
| `-i`   | Your callsign / ID                 | `irmc-default`           |
| `-s`   | Serial port device for telegraph key | *(none)*               |
| `-?`   | Show help                          |                          |

### Examples

```bash
# Connect with defaults (mtc-kob.dyndns.org:7890, channel 103)
./irmc

# Join channel 205 on a specific server
./irmc -h morsecode.dc3.com -c 205

# Identify yourself and attach a serial key
./irmc -h morsecode.dc3.com -c 113 -i W1AW -s /dev/ttyUSB0

# Use a local test server
./irmc -h localhost -c 100
```

---

## The CWCom Protocol

IRMC implements the **CWCom** protocol, an open UDP-based standard for transmitting morse code timing data between clients and relay servers. The protocol is documented in detail in [`doc/cwcom.pdf`](doc/cwcom.pdf).

### Transport

- **Protocol:** UDP (`SOCK_DGRAM`)
- **Default server:** `mtc-kob.dyndns.org`
- **Default port:** `7890`
- Clients connect to a numbered **channel** — everyone on the same channel hears each other.
- A **keep-alive** packet (ID + connect) is sent every ~5 seconds to maintain the session.

### Packet Types

The protocol uses two packet formats:

#### Command Packet (4 bytes)

Used for connecting and disconnecting.

```
Offset  Size   Field     Description
──────  ─────  ────────  ──────────────────────────────
0x00    2      command   CON (0x0004) or DIS (0x0002)
0x02    2      channel   Channel number
```

#### Data Packet (496 bytes)

Used for identification, morse data transmission, and latch/unlatch signals.

```
Offset  Size   Field      Description
──────  ─────  ─────────  ──────────────────────────────────────────
0x00    2      command    DAT (0x0003)
0x02    2      length     Payload length (always 492)
0x04    128    id         Sender callsign / identifier (null-terminated)
0x84    4      a1         Reserved
0x88    4      sequence   Packet sequence number (monotonically increasing)
0x8C    4      a21        Magic number (1 for ID packets, 0 for TX)
0x90    4      a22        Magic number (755)
0x94    4      a23        Magic number (65535 for ID, 16777215 for TX)
0x98    204    code[51]   Morse timing array (51 × signed 32-bit int)
0x164   4      n          Number of valid elements in code[]
0x168   128    status     Decoded character or interface version string
0x1E8   8      a4         Reserved
```

**Total: 496 bytes** — every data packet is exactly this size, regardless of content.

### Command Codes

| Code     | Value    | Description                                    |
|:---------|:---------|:-----------------------------------------------|
| `DIS`    | `0x0002` | Disconnect from channel                        |
| `DAT`    | `0x0003` | Data packet (morse timing, ID, latch/unlatch)  |
| `CON`    | `0x0004` | Connect to channel                             |
| `ACK`    | `0x0005` | Acknowledgement                                |

### Morse Timing Encoding

Morse code is encoded as an array of **signed 32-bit integers** in the `code[]` field:

| Value       | Meaning                                        |
|:------------|:-----------------------------------------------|
| **> 0**     | **Mark** (key down / tone) — duration in ms    |
| **< 0**     | **Space** (key up / silence) — duration in ms  |
| **= 1**     | Special: **latch** signal (close circuit)      |
| **= 2**     | Special: **unlatch** signal (open circuit)     |
| **= 0**     | No-op (ignored)                                |

For example, the letter "A" (dit-dah) might be encoded as:

```
code[] = { 60, -60, 180 }     n = 3
           ↑    ↑    ↑
          dit  gap  dah
```

Values with `abs(value) > 2000` are treated as invalid and ignored.

### Session Lifecycle

```
Client                              Server
  │                                    │
  │──── CON(channel) ─────────────────>│   1. Connect to channel
  │──── DAT(id_packet) ──────────────>│   2. Send identification
  │                                    │
  │<─── DAT(morse data) ─────────────│   3. Receive morse from others
  │──── DAT(morse data) ─────────────>│   4. Transmit own morse
  │                                    │
  │──── CON + DAT(id) ───────────────>│   5. Keep-alive (every ~5s)
  │         ...                        │
  │──── DIS ──────────────────────────>│   6. Disconnect
  │                                    │
```

Each data packet is sent **5 times** redundantly (UDP has no delivery guarantee).

### TX Timeout

If no morse key activity is detected for **240 seconds**, the transmitter loop returns to idle. This prevents accidental channel blocking.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         irmc.c                              │
│              (main loop, CLI, networking)                    │
│                                                             │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────────┐ │
│  │ cwprotocol.c │  │   beep.c     │  │     util.c        │ │
│  │              │  │              │  │                   │ │
│  │ Packet build │  │ PortAudio    │  │ fastclock()       │ │
│  │ Connect/Disc │  │ sine wave    │  │ kbhit()           │ │
│  │ ID / TX / RX │  │ tone output  │  │ get_in_addr()     │ │
│  └──────────────┘  └──────────────┘  └───────────────────┘ │
└─────────────────────────────────────────────────────────────┘
         │                  │                    │
    UDP Socket         PortAudio            POSIX / Mach
    (network)           (audio)             (time, I/O)
```

### Modules

| File              | Lines | Responsibility                                              |
|:------------------|------:|:------------------------------------------------------------|
| `irmc.c`          |   382 | Entry point, CLI parsing, UDP socket setup, main event loop, signal handling, TX/RX logic |
| `cwprotocol.c/h`  |   147 | CWCom packet construction (connect, disconnect, data, ID), global packet state, latch/unlatch |
| `beep.c/h`        |   203 | Audio output via PortAudio — square wave callback at 44.1 kHz sample rate, frequency/duration control |
| `util.c/h`        |    87 | Cross-platform millisecond timer (`fastclock`), non-blocking keyboard input (`kbhit`), IPv4/v6 address helper |

### Main Loop Flow

The core of IRMC is an infinite loop in `main()` that runs roughly every 50 microseconds:

1. **Receive** — Read incoming UDP packets (non-blocking)
2. **Decode** — If a 496-byte data packet arrived with new sequence number:
   - Display decoded text (`status` field)
   - Play back morse timing via audio (`beep()`)
   - Handle latch/unlatch signals
3. **Transmit** — If local key input produced morse timing data:
   - Send the data packet 5× redundantly
4. **Key input** — Poll serial port (DSR line) or GPIO pin for key state changes
5. **Keep-alive** — Every ~100 iterations, re-send connect + ID packets
6. **Flush** — Discard any stale keyboard input

### Conditional Compilation

| Flag       | Effect                                               |
|:-----------|:-----------------------------------------------------|
| `RASPI`    | Enable Raspberry Pi GPIO input via wiringPi (pin 5), adjust audio latency ×6 |
| `TX_SERIAL`| Enable serial port key input (currently disabled in source) |
| `DEBUG`    | Print received packet fields and TX events to stdout |
| `__MACH__` | Use macOS Mach clock instead of `clock_gettime`      |

---

## Hardware Interfaces

A telegraph key or paddle can be connected via RS-232 serial or Raspberry Pi GPIO. For detailed build instructions, see the [MorseKOB interface guide](http://kob.sdf.org/morsekob/interface.htm) and the included [loop interface PDF](doc/loopinterface.pdf).

### RS-232 Serial (DB9)

| RS232 Signal | DB9 Pin | Function                    |
|:-------------|:--------|:----------------------------|
| DTR          | 4       | Manual key / paddle common  |
| DSR          | 6       | Manual key / dot paddle     |
| CTS          | 8       | Dash paddle                 |
| RTS          | 7       | Sounder output              |
| SG           | 5       | Ground                      |

### Raspberry Pi GPIO

When built with `make raspi`, IRMC reads key input from **GPIO pin 5** (wiringPi numbering). Connect your key between GPIO 5 and ground. The pin is configured as `INPUT` with the wiringPi library.

---

## Testing

There is no automated test suite. Testing is manual:

```bash
# Start the included local test server (requires Java)
cd src && make java

# In another terminal, connect to it
./irmc -h localhost -c 100
```

Or connect to a public server:

```bash
./irmc -h morsecode.dc3.com -c 205
```

---

## Changelog

| Version | Changes                                          |
|:--------|:-------------------------------------------------|
| v0.3.3  | Current — minor fixes and cleanups               |
| v0.3    | Command-line option cleanup                      |
| v0.2    | Ported to Debian Wheezy and OS X Yosemite (DG6FL)|
| v0.1    | Original version (VE4FEB)                        |

---

## Authors

- **Fernan Bolando** (VE4FEB) — Original author
- **Gerolf Ziegenhain** (DG6FL) — Linux/macOS port, ongoing development
- **Les Kerr** — MorseKOB server and protocol
- **John Samien** (VK1EME) — CWCom protocol

---

## References

- [MOIP — Morse Code over IP](http://morse-code-over-ip.github.io/moip/) — Protocol specification
- [MorseKOB](http://kob.sdf.org/) — Server software and interface guides
- [CWCom Protocol Spec](doc/cwcom.pdf) — Detailed packet documentation (PDF)
- [Loop Interface Guide](doc/loopinterface.pdf) — Hardware telegraph loop interface (PDF)
