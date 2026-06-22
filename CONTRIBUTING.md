# Contributing to LabKit ESP32 Firmware

Thank you for helping improve the firmware. This project is open source under the [MIT License](LICENSE).

## Quick start

1. **Fork** [labkitapp/labkit_esp32](https://github.com/labkitapp/labkit_esp32) on GitHub.
2. **Clone** your fork and add the upstream remote:
   ```bash
   git clone https://github.com/YOUR_USER/labkit_esp32.git
   cd labkit_esp32
   git remote add upstream https://github.com/labkitapp/labkit_esp32.git
   ```
3. **Branch** from `develop` (not `main`):
   ```bash
   git checkout develop
   git pull upstream develop
   git checkout -b feature/short-description
   ```
4. **Build and test** on an **ESP32 DevKit-C** with `ESP32DEVKIT` in `board_define.h` (see [README](README.md)).
5. **Commit** with a clear message describing *why* the change helps.
6. **Push** to your fork and open a **pull request** into `labkitapp/labkit_esp32` **`develop`**.

## What we merge

- Bug fixes and reliability improvements
- Documentation fixes
- Features that fit the DevKit-C target and existing architecture
- Changes tested on real hardware when behavior is affected

Please keep each pull request focused on one logical change.

## What to avoid

- Committing build outputs (`build/`, `*.bin`, `*.elf`, `*.map`) — these are gitignored
- Large unrelated refactors mixed with feature work
- Pull requests targeting **`main`** (releases are cut from `develop` by maintainers)

## Build requirements

- [Arduino IDE 2](https://www.arduino.cc/en/software) with the **ESP32** board package, or `arduino-cli`
- Libraries: **ArduinoJson**, **ESP Async WebServer**, **Async TCP**
- Board: **ESP32 Dev Module**, 4 MB flash, default partition scheme

## Pull request checklist

- [ ] Branch is based on latest `develop`
- [ ] `board_define.h` uses `ESP32DEVKIT` unless the PR explicitly targets another board (discuss first)
- [ ] Tested on hardware when firmware behavior changed
- [ ] README or comments updated if user-facing behavior changed
- [ ] No secrets, WiFi credentials, or personal paths in the diff

## Reporting bugs

Open a [GitHub Issue](https://github.com/labkitapp/labkit_esp32/issues) with:

- Board model and flash size
- Steps to reproduce
- Expected vs actual behavior
- Serial log excerpt if relevant

## Releases

Maintainers merge `develop` → `main` and tag releases (e.g. `v1.1`). Public browser flashes on [labkit.app](https://labkit.app/release.html?type=project&id=05e65af712e13f57) are updated separately after a release tag.

## Questions

For platform questions beyond firmware, see [labkit.app](https://labkit.app) and the [LabKit GitHub org](https://github.com/labkitapp).
