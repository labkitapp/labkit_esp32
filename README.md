# LabKit ESP32 Firmware

Arduino firmware for the **ESP32 DevKit-C**. Turns a cheap dev board into a browser-controlled hardware debug tool — GPIO, I2C, SPI, UART, and analog — with an on-device web UI and WebSocket console.

**Current release:** `v1.0` · **On-device version:** `1.0`

| | |
|---|---|
| **Flash in browser** | [![Flash with LabKit](https://img.shields.io/badge/Flash%20with-LabKit-4ade80?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCI+PHBhdGggZmlsbD0id2hpdGUiIGQ9Ik0xMyAyTDMgMTRoOWwtMSA4IDEwLTEySDE0bDEtOHoiLz48L3N2Zz4=&style=flat)](https://labkit.app/release.html?type=project&id=05e65af712e13f57) |
| **Product page** | [labkit.app/apps/devkit-c](https://labkit.app/apps/devkit-c) |
| **Platform** | [labkit.app](https://labkit.app) |

No toolchain required to flash — use Chrome or Edge, connect the board over USB, and click **Flash**.

---

## Features

- **On-device web UI** — configure pins, run bus transactions, and monitor state from the browser
- **WebSocket console** — real-time serial bridge between the browser and UART
- **I2C** — bus scan, register read/write, device discovery
- **SPI** — configurable transfers via the web interface
- **UART** — baud-rate control and message passthrough
- **GPIO & analog** — pin direction, digital I/O, and analog reads (DevKit target)
- **WiFi provisioning** — credentials stored in flash; connect via UART command or the web UI

---

## Hardware

| Item | Detail |
|------|--------|
| **Target board** | ESP32 DevKit-C (4 MB flash) |
| **Board define** | `ESP32DEVKIT` in `board_define.h` |
| **Default I2C** | SDA `21`, SCL `22` |
| **Default UART1** | RX `17`, TX `16` |
| **USB** | Required for flashing and serial console |

---

## Flash firmware

### Option A — Browser (recommended)

Click the badge above or open the [LabKit release page](https://labkit.app/release.html?type=project&id=05e65af712e13f57). Connect your ESP32 over USB and flash — no Arduino IDE or esptool needed.

Works on **Chrome** and **Edge** (Windows, macOS, Linux).

### Option B — Build from source

1. Install [Arduino IDE 2](https://www.arduino.cc/en/software) with the **ESP32** board package.
2. Install libraries: **ArduinoJson**, **ESP Async WebServer**, **Async TCP**.
3. Open `labkit_esp32.ino`.
4. Confirm `board_define.h` has `#define ESP32DEVKIT`.
5. Select board **ESP32 Dev Module** (4 MB flash, default partition scheme).
6. Upload via USB.

Or compile with `arduino-cli`:

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32:FlashSize=4M,PartitionScheme=default,FlashMode=qio,FlashFreq=80 \
  --output-dir build \
  labkit_esp32
```

Build artifacts (`build/`, `*.bin`, `*.elf`, `*.map`) are gitignored.

---

## First run

1. Flash the firmware and open the serial monitor (**115200** baud).
2. Connect to WiFi — send over serial:
   ```
   wifi <ssid> <password>
   ```
   Credentials are saved to flash and reused on boot.
3. When connected, the serial output prints the device IP. Open `http://<ip>` in a browser to use the on-device UI.

Scan for nearby networks:
```
wifi scan
```

---

## Project layout

| File / folder | Purpose |
|---------------|---------|
| `labkit_esp32.ino` | Main sketch — setup, WiFi, HTTP/WebSocket server |
| `Config.h` | Version string, feature flags, buffer sizes |
| `board_define.h` | Hardware target selection (`ESP32DEVKIT`) |
| `index_html.h` | Bundled on-device web UI |
| `*Manager.*` | I2C, SPI, pin, and error handling modules |
| `WebSocketCommandHandler.*` | WebSocket protocol and commands |
| `JsonResponseBuilder.*` | JSON API responses |
| `CommandHandlers.*` | UART and command dispatch |

---

## Branches & releases

| Branch / tag | Purpose |
|--------------|---------|
| `main` | Production line — tagged releases only |
| `develop` | Active firmware development |
| `v1.0` | Initial public source release |

---

## Related

- **LabKit platform** — [github.com/labkitapp](https://github.com/labkitapp)
- **Release docs** — [labkit.app/releases/docs](https://labkit.app/releases/docs) (sharing links, embed widgets, README badges)

---

## Contributing

Contributions are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for the fork → PR workflow. **Open pull requests against `develop`, not `main`.**

---

## License

This firmware is licensed under the [MIT License](LICENSE).

Copyright © 2026 LabKit
