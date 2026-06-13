# libFRX
Driver Library for the open FRX audio recording ecosystem

**Platform:** ESP32 (Espressif32)
**Status:** Active Development

## 📋 Overview

libFRX is a **modular, configurable driver library** for FRX ecosystem devices (FR1-mini, FR2, etc.). It provides a unified codebase with instance-specific configuration through **undefined macros** that consuming projects define in their `platformio.ini` files.

### Key Features
- ✅ **Modular Design**: Each component is self-contained and configurable
- ✅ **Cross-Device Compatibility**: Works with FR1-mini, FR2, and future FRX devices
- ✅ **Configuration via Macros**: Instance-specific settings without code duplication
- ✅ **Comprehensive Testing**: Unit tests for all modules
- ✅ **PlatformIO Integration**: Seamless integration with PlatformIO build system

## 🗺️ Project Structure

```
libFRX/
├── lib/                          # Core library modules
│   ├── audio/                    # I2S audio interface
│   ├── dsp_frx/                  # Digital signal processing
│   ├── ext_flash/                # External flash memory
│   ├── i2c/                      # I2C communication
│   ├── sdcard/                   # SD card interface
│   ├── synth/                    # Audio synthesis
│   ├── syserr/                   # System error codes
│   ├── utils/                    # Utility functions
│   └── wav/                      # WAV file handling
│
├── test/                         # Unit tests
│   └── test_<module>/            # Module-specific test suites
│
├── shared/                       # Shared resources
│   └── scripts/                 # Build and utility scripts
│
├── src/                          # Demo applications
│   └── sdcard_demo.cpp           # SD card CLI demo
│
├── library.json                  # PlatformIO library manifest
├── platformio.ini                # PlatformIO configuration
├── README.md                     # This file
└── .gitignore
```

### Module Organization

Each module follows this structure:
```
lib/<module_name>/
├── <module_name>.h          # Public interface with configuration macros
├── <module_name>.cpp       # Implementation
├── <module_name>_jccl.h    # CLI command definitions (if applicable)
└── <module_name>_cfg.h     # Default configuration (if complex)
```

## 🔧 Configuration System

libFRX uses **undefined macros** for instance-specific configuration. Projects define these in their `platformio.ini` build flags.

### Configuration Pattern

```ini
[env:my_device]
platform = espressif32
framework = arduino
build_flags =
    ; Audio configuration
    -DAUDIO_I2S_PORT=0
    -DAUDIO_PIN_MEMS_I2S_BCLK=15
    -DAUDIO_PIN_MEMS_I2S_WS=17
    -DAUDIO_PIN_MEMS_I2S_IN=16
    
    ; I2C configuration
    -DI2C_BASE_SCL=21
    -DI2C_BASE_SDA=20
    -DI2C_BASE_SPEED=100000
    
    ; SDCard configuration
    -DSDCARD_MODE=sd_mode_spi
    -DSDCARD_SPI_PIN_CS=13

lib_deps =
    git@github.com:jesdev-io/libFRX.git
```

### Configuration Philosophy

1. **Sensible Defaults**: Each module provides defaults (typically FR1-mini or FR2 values)
2. **Override Only What's Different**: Projects only need to define macros that differ from defaults
3. **Compile-Time Configuration**: All settings are resolved at compile time
4. **No Runtime Overhead**: Configuration doesn't affect performance

## 📦 Available Modules

| Module | Description | Status |
|--------|-------------|--------|
| **audio** | I2S audio interface with configurable sample types and channels | ✅ Stable |
| **dsp_frx** | DSP functions: sine approx, mean square, dBFS conversion, rolling average | ✅ Stable |
| **ext_flash** | W25Qxx external flash driver with SPI interface | ✅ Stable |
| **i2c** | I2C base driver with mutex locking | ✅ Stable |
| **sdcard** | Unified SPI + SDMMC support with mode selection | ✅ Stable |
| **synth** | Synthesizer (sine, square, saw, sweep) | ✅ Stable |
| **syserr** | System error codes (header-only) | ✅ Stable |
| **utils** | Utility functions (strremove, uint/str conversion) | ✅ Stable |
| **wav** | WAV file handling | ✅ Stable |

## 🚀 Usage

### 1. Add Dependency

Add libFRX to your `platformio.ini`:

```ini
lib_deps =
    git@github.com:jesdev-io/libFRX.git
```

### 2. Configure Macros

Define required macros in your project's `platformio.ini` build flags (see Configuration Pattern above).

### 3. Include Headers

```cpp
#include <libFRX/lib/audio/audio.h>
#include <libFRX/lib/dsp_frx/dsp_frx.h>
#include <libFRX/lib/sdcard/sdcard.h>
// ... other modules as needed
```

### 4. Initialize and Use

```cpp
void setup() {
    // Initialize modules with your configuration
    audio_init(AUDIO_I2S_PORT, AUDIO_PIN_MEMS_I2S_BCLK, ...);
    sd_init_default();
    
    // Use module functionality
    audio_start_recording();
    sd_mnt();
}
```

## 🧪 Testing

### Running Tests

**Embedded Tests:**
```bash
# Run tests with upload
platformio test --environment frx_test_sdcard

# Run tests without upload (device must be pre-flashed)
platformio test --environment frx_test_sdcard --without-uploading

# Build test firmware only (no upload, no run)
platformio test --environment frx_test_sdcard --without-uploading --without-testing
```

**Python CLI Tests:**
```bash
# Flash demo firmware first
platformio run --target upload --environment frx_demo_sdcard

# Run Python tests
uv run -m pytest tests/test_sdcard_cli.py -v
```

### Test Environments

| Environment | Purpose | Upload Port |
|------------|---------|-------------|
| `frx_test_sdcard` | SD card module tests | `/dev/ttyACM0` |
| `frx_test_audio` | Audio module tests | `/dev/ttyACM0` |
| `frx_demo_sdcard` | SD card CLI demo | `/dev/ttyACM0` |

### Test Structure

```
test/<module>/
├── test_<module>_fw.cpp      # Embedded test cases
├── test_<module>_hw.cpp      # Hardware-specific tests (if needed)
└── test_<module>_utils.cpp   # Utility functions for tests
```

## 🛠️ Development

### Adding a New Module

1. **Create Module Directory**:
   ```bash
   mkdir -p libFRX/lib/<module_name>
   ```

2. **Add Header File**: Define public interface and configuration macros
   ```cpp
   // lib/<module_name>/<module_name>.h
   #ifndef LIBFRX_<MODULE>_H
   #define LIBFRX_<MODULE>_H
   
   #include <Arduino.h>
   #include <jescore.h>
   
   // Configuration macros with defaults
   #ifndef <MODULE>_PIN_X
   #define <MODULE>_PIN_X 21  // Default pin
   #endif
   
   // Function declarations
   void <module>_init();
   e_syserr_t <module>_do_something();
   
   #endif // LIBFRX_<MODULE>_H
   ```

3. **Add Implementation**: Implement functionality in `.cpp` file

4. **Add CLI Support (Optional)**: Create `<module_name>_jccl.h` for CLI commands

5. **Update Library Manifest**: Add to `library.json`

6. **Add Tests**: Create test files in `test/test_<module>/`

### Module Design Guidelines

✅ **DO:**
- Use undefined macros for all configurable parameters
- Provide sensible defaults in header files
- Keep dependencies minimal
- Use generic types (e.g., `uint32_t` instead of device-specific types)
- Document all configurable macros
- Follow existing code style and patterns

❌ **DON'T:**
- Use hardcoded pins or settings
- Add device-specific code (use macros instead)
- Create circular dependencies between modules
- Use dynamic memory allocation (use jescore's memory management)

### Build System

**PlatformIO Configuration:**
- `platformio.ini`: Main configuration
- `library.json`: Library manifest for PlatformIO
- Build flags are inherited by consuming projects

**Custom Build Scripts:**
- `shared/scripts/jccl_macro_parser.py`: Generates Python test strings from CLI headers
- `shared/scripts/get_time.py`: Build timestamp script

## 📋 Module Porting Status

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

## 🔗 Dependencies

- **Platform:** Espressif32 (ESP32)
- **Frameworks:** Arduino-ESP32
- **Core Dependency:** jescore >= 2.3.0
- **Build System:** PlatformIO

## 📄 License

**Apache License 2.0**

See [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork the repository** and create a feature branch
2. **Follow the existing code style** and patterns
3. **Add tests** for new functionality
4. **Update documentation** (README, module headers)
5. **Submit a pull request** with clear description

### Issue Tracking

- 🐛 **Bug Reports**: Use GitHub Issues with clear reproduction steps
- 🚀 **Feature Requests**: Use GitHub Issues with use case description
- ❓ **Questions**: Use GitHub Discussions

## 📬 Contact

- **Project Website**: [jesdev.io](https://jesdev.io)
- **GitHub**: [github.com/jesdev-io/libFRX](https://github.com/jesdev-io/libFRX)
- **Issues**: [github.com/jesdev-io/libFRX/issues](https://github.com/jesdev-io/libFRX/issues)

---

*libFRX is part of the open FRX audio ecosystem - building accessible, open-source audio recording tools.*
