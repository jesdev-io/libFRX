# libFRX
Driver Library for the open FRX audio recording ecosystem

**Platform:** ESP32 (Espressif32)

## Overview

libFRX is a modular shared library containing common drivers and utilities for FRX ecosystem devices (FR1-mini, FR2, etc.). Each module is designed to be instance-specific configurable via **undefined macros** that consuming projects define in their `platformio.ini` files.

libFRX is a **PlatformIO library project** that can be used as a dependency in other PlatformIO projects.

## Configuration Pattern

libFRX modules use **undefined macros** for instance-specific configuration. Projects (FR1-mini, FR2) define these macros in their `platformio.ini` build flags. If not defined, default values (typically from FR1-mini or FR2) are used.

### Example: Multi-Module Configuration

In your project's `platformio.ini`:

```ini
[env:my_environment]
platform = espressif32
framework = arduino
build_flags =
    ; Audio configuration
    -DAUDIO_I2S_PORT=0                ; I2S_NUM_0
    -DAUDIO_PIN_MEMS_I2S_BCLK=15     ; BCLK pin
    -DAUDIO_PIN_MEMS_I2S_WS=17        ; WS pin
    -DAUDIO_PIN_MEMS_I2S_IN=16        ; Data IN pin
    -DAUDIO_MAX_NUM_CH=2              ; Max channels (default: 2)
    
    ; I2C configuration
    -DI2C_BASE_SCL=21                 ; SCL pin (FR2 default)
    -DI2C_BASE_SDA=20                 ; SDA pin (FR2 default)
    -DI2C_BASE_NUM=0                  ; I2C_NUM_0
    -DI2C_BASE_SPEED=100000           ; 100 kHz
    
    ; External Flash configuration (FR2 defaults)
    -DEF_PIN_MISO=46
    -DEF_PIN_MOSI=11
    -DEF_PIN_SCLK=12
    -DEF_PIN_CS=14
    -DEF_SPI_HOST=1                   ; SPI2_HOST
    
    ; SDCard configuration
    -DSDCARD_MODE=sd_mode_spi         ; or sd_mode_sdmmc, sd_mode_auto
    -DSDCARD_SPI_PIN_CS=13
    -DSDCARD_SPI_PIN_MOSI=15
    -DSDCARD_SPI_PIN_MISO=2
    -DSDCARD_SPI_PIN_SCK=14
    
    ; DSP configuration (FR1-mini defaults)
    -DDSP_FRX_MIC_SENS=-26            ; Microphone sensitivity in dB
    -DDSP_FRX_DC_FILTER_ALPHA=0.001   ; DC filter alpha
    -DDSP_FRX_ROLL_AVG_N=10           ; Rolling average window size
    
    ; Synth configuration
    -DSYNTH_SWEEP_DUR_S=1
    -DSYNTH_SWEEP_LEN=48000

lib_deps =
    git@github.com:jesdev-io/libFRX.git
```

Modules provide defaults for all macros, so projects only need to override what differs from their defaults.

### Available Modules

| Module | Source | Status | Description |
|--------|--------|--------|-------------|
| **audio** | FR1-mini | ✅ | I2S audio interface with configurable sample types and channels |
| **dsp_frx** | FR1-mini (renamed from dsp_fr1) | ✅ | DSP functions: sine approx, mean square, dBFS conversion, rolling average |
| **ext_flash** | FR2 | ✅ | W25Qxx external flash driver with SPI interface |
| **i2c** | FR2 | ✅ | I2C base driver with mutex locking |
| **sdcard** | FR1-mini + FR2 | ✅ | Unified SPI + SDMMC support with mode selection |
| **synth** | FR2 | ✅ | Synthesizer (sine, square, saw, sweep) |
| **syserr** | FR1-mini/FR2 | ✅ | System error codes (header-only) |
| **utils** | FR1-mini | ✅ | Utility functions (strremove, uint/str conversion) |
| **wav** | FR1-mini | ✅ | WAV file handling |
| **shared/scripts/get_time.py** | FR2 | ✅ | PlatformIO build time script |

## Module Porting Status (from todo.md)

| Priority | Module | Source | Status |
|----------|--------|--------|--------|
| 1 | audio | FR1-mini | ✅ **DONE** |
| 2 | dsp_fr1 | FR1-mini | ✅ **DONE** (renamed to dsp_frx) |
| 3 | ext_flash | FR2 | ✅ **DONE** |
| 4 | i2c | FR2 | ✅ **DONE** |
| 5 | sdcard | FR1-mini + FR2 | ✅ **DONE** - Unified SPI + SDMMC |
| 6 | synth | FR2 | ✅ **DONE** |
| 7 | syserr | FR1-mini | ✅ **DONE** |
| 8 | utils | FR1-mini | ✅ **DONE** |
| 9 | wav | FR1-mini | ✅ **DONE** |
| 10 | shared/scripts/get_time.py | FR2 | ✅ **DONE** |

## Usage

1. Add libFRX as a dependency in your `platformio.ini`:
   ```ini
   lib_deps =
       git@github.com:jesdev-io/libFRX.git
   ```

2. Define required macros in your project's `platformio.ini` build flags (see Configuration Pattern above).

3. Include module headers in your code:
   ```cpp
   #include <libFRX/lib/audio/audio.h>
   #include <libFRX/lib/dsp_frx/dsp_frx.h>
   #include <libFRX/lib/ext_flash/ext_flash.h>
   #include <libFRX/lib/i2c/i2c_base.h>
   #include <libFRX/lib/sdcard/sdcard.h>
   #include <libFRX/lib/synth/synth.h>
   #include <libFRX/lib/syserr/syserr.h>
   #include <libFRX/lib/utils/utils.h>
   #include <libFRX/lib/wav/wav.h>
   ```

## Development

### Project Structure

libFRX is organized as a PlatformIO library project:

```
libFRX/
├── lib/
│   ├── audio/
│   │   ├── audio.h
│   │   └── audio.cpp
│   ├── dsp_frx/
│   │   ├── dsp_frx.h
│   │   └── dsp_frx.cpp
│   ├── ext_flash/
│   │   ├── ext_flash.h
│   │   └── ext_flash.cpp
│   ├── i2c/
│   │   ├── i2c_base.h
│   │   └── i2c_base.cpp
│   ├── sdcard/
│   │   ├── sdcard.h
│   │   └── sdcard.cpp
│   ├── synth/
│   │   ├── synth.h
│   │   ├── synth.cpp
│   │   └── sweep_values.inc
│   ├── syserr/
│   │   └── syserr.h
│   ├── utils/
│   │   ├── utils.h
│   │   └── utils.cpp
│   └── wav/
│       ├── wav.h
│       └── wav.cpp
├── shared/
│   └── scripts/
│       └── get_time.py
├── library.json          ; PlatformIO library manifest
├── platformio.ini        ; PlatformIO project configuration
├── README.md
└── .gitignore
```

### Adding a New Module

1. Create module directory under `libFRX/lib/<module_name>/`
2. Add header and implementation files
3. Use `#ifndef` guards for all configurable parameters with sensible defaults
4. Add module to `library.json` headers and build flags
5. Update this README

### Module Design Guidelines

- **Use undefined macros** for all instance-specific configuration (pins, speeds, etc.)
- **Provide sensible defaults** in the header file (typically FR1-mini or FR2 values)
- **Keep dependencies minimal** - depend on syserr.h and jescore where needed
- **Use generic types** where possible (e.g., `ext_flash_rom_t` instead of `fr2_rom_t`)
- **Document all configurable macros** in the header file

### Testing

Use PlatformIO test environments:
```bash
cd libFRX
platformio test --environment <env> --upload-port /dev/ttyACM0 --test-port /dev/ttyACM0
```

See `test/` directory for module-specific test files.

## Platform Support

- **Primary Platform:** Espressif32 (ESP32)
- **Frameworks:** Arduino-ESP32
- **Dependencies:** jescore >= 2.3.0

## License

Apache-2.0
