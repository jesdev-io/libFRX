# libFRX

Driver and field-recorder utility library for ESP32-based FRX devices.

`libFRX` provides reusable modules for building field recorders: audio I/O,
SD-card storage, WAV files, DSP helpers, synthesis, external flash, I2C, and
jescore-compatible CLI control. A concrete firmware chooses which modules to
enable and supplies board-specific pins and policy.

## Intended split

- `lib/`: reusable driver and field-recorder modules.
- `src/`: demo or device-specific firmware entry points.
- `test/`: hardware-backed PlatformIO tests for this repository.
- `docs/`: user-facing reference and design notes.

## Add to a PlatformIO project

`libFRX` is designed to be most convenient in PlatformIO, where module enable flags and board-specific macros can live in one environment. It can also be used in a classic ESP-IDF/Arduino build by adding the sources and defining the same macros yourself, but this repository does not maintain a separate guide for that path.

```ini
lib_deps =
    https://github.com/jesdev-io/libFRX.git
```

Enable only the modules your firmware uses:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_AUDIO
    -DFRX_ENABLE_MODULE_SDCARD
    -DFRX_ENABLE_MODULE_WAV
```

Then define the board-specific macros required by those modules. For example,
a stereo audio + SDMMC recorder might define:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_AUDIO
    -DFRX_ENABLE_MODULE_SDCARD
    -DFRX_ENABLE_MODULE_WAV

    ; Audio pins/topology
    -DAUDIO_BANKS_CFG_DEFAULT=AUDIO_BANKS_CFG_SINGLE_STEREO_IO
    -DAUDIO_PIN_I2S_BCLK=15
    -DAUDIO_PIN_I2S_WS=17
    -DAUDIO_PIN_I2S_IN_A=16
    -DAUDIO_PIN_I2S_OUT_A=6

    ; SD card in SDMMC mode
    -DSDCARD_MODE_SDMMC
    -DSDCARD_SDMMC_PIN_CLK=36
    -DSDCARD_SDMMC_PIN_CMD=35
    -DSDCARD_SDMMC_PIN_D0=37
    -DSDCARD_SDMMC_PIN_D1=38
    -DSDCARD_SDMMC_PIN_D2=45
    -DSDCARD_SDMMC_PIN_D3=39
```

## Available modules

| module | Purpose |
|---|---|
| `audio` | ESP32 I2S audio I/O with realtime callback seam and CLI control |
| `sdcard` | SD-card mount, file, directory, and streaming helpers |
| `wav` | WAV header/file lifecycle helpers for recorded audio |
| `dsp_frx` | Lightweight DSP helpers for field-recorder measurements |
| `synth` | Sine, square, saw, and sweep generation into audio buffers |
| `ext_flash` | W25Qxx-style external SPI flash access |
| `i2c` | I2C master base module with scan support |
| `utils` | Small utility helpers |
| `syserr` | Shared error enum |

## Documentation

User-facing docs are built with MkDocs Material and live in `docs/`:

- [Docs home](docs/index.md)
- [API reference](docs/reference/index.md)
- [Audio design/API notes](docs/reference/audio.md)

## Contributing

Repository maintenance workflows, docs generation, local wiki preview, testing,
and generated-file rules are documented in [CONTRIBUTING.md](CONTRIBUTING.md).

## License

Apache License 2.0. See [LICENSE](LICENSE).
