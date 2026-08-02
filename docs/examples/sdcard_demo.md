# SD card demo application

Source: `src/sdcard_demo.cpp`

This is a `jescore` compatibility-layer demo for the SD-card module. The firmware
initializes `jescore`, initializes the SD-card module with its default
configuration, and then waits for CLI calls.

The firmware entry point is deliberately small:

```cpp
void setup(){
    jes_init();
    sd_init_default();
}
```

`loop()` does not run application logic. Control happens through the SD-card
`jescore` job, which can be called from `jescorecli`.

## Required configuration

The demo uses `sd_init_default()`, so the build environment must enable the
SD-card module and choose either SDMMC or SPI mode with the required pins.

Example SDMMC configuration:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_SDCARD
    -DSDCARD_MODE_SDMMC
    -DSDCARD_SDMMC_PIN_CLK=36
    -DSDCARD_SDMMC_PIN_CMD=35
    -DSDCARD_SDMMC_PIN_D0=37
    -DSDCARD_SDMMC_PIN_D1=38
    -DSDCARD_SDMMC_PIN_D2=45
    -DSDCARD_SDMMC_PIN_D3=39
```

The same SD-card module can also use normal SPI instead of SDMMC:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_SDCARD
    -DSDCARD_MODE_SPI
    -DSDCARD_SPI_PIN_CS=13
    -DSDCARD_SPI_PIN_MOSI=15
    -DSDCARD_SPI_PIN_MISO=2
    -DSDCARD_SPI_PIN_SCK=14
```

The home directory is set by `SDCARD_BASE_PATH`. Output size for listing and
file printing is controlled by `SDCARD_LS_MAX_CHAR` and `SDCARD_CAT_MAX_CHAR`.

See [Configuration and defaults](../reference/configuration.md) for the general
build-flag model.

## CLI calls

Install `jescorecli` from <https://github.com/jesdev-io/jescorecli> and connect
to the device.

Available SD-card calls:

| Call | Effect |
|---|---|
| `sdcard mnt` | Mount the SD card. |
| `sdcard unmnt` | Unmount the SD card. |
| `sdcard ls` | List files under `SDCARD_BASE_PATH`. |
| `sdcard cat <file>` | Read and print text content of a file. |
| `sdcard mk <file>` | Create an empty file. |
| `sdcard rm <file>` | Remove a file. This is irreversible. |
| `sdcard mem` | Print free and total card memory. |

## What this example proves

This demo shows the intended split between reusable library code and firmware
entry point:

- `lib/sdcard/` owns mount/unmount, file operations, directory listing, and CLI
  command handling;
- `src/sdcard_demo.cpp` only initializes `jescore` and the SD-card module.

Use this as the smallest firmware shape for checking SD-card wiring and CLI
access before composing SD storage with audio or WAV recording.
