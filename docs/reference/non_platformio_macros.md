# Non-PlatformIO macro inputs

If you build the same firmware with another build system, pass the same preprocessor definitions that
PlatformIO would pass through `build_flags` or its pre-scripts.

This page lists the repo-level macros that are easy to miss because they are not
owned by a single module header.

## Base repository macros

The common `_env` section in `platformio.ini` defines these for repository
firmware and tests:

| Macro | Example | Purpose |
|---|---:|---|
| `JES_LOG_LEN` | `16` | `jescore` log/message buffer sizing used by repository firmware. |
| `FRX_FW_VER_MAJ` | `0` | Firmware major version used by firmware/test metadata. |
| `FRX_FW_VER_MIN` | `1` | Firmware minor version used by firmware/test metadata. |
| `FRX_FW_VER_MOD` | `97` | Firmware modifier/build letter used by firmware/test metadata. |
| `FR1_SER_NUM` | `0` | Device serial-number placeholder for repository builds. |

If your non-PlatformIO firmware does not use the metadata/test paths that read
these values, you can choose your own equivalents or omit the unused ones. If you
want parity with the repository environments, define all of them.

## Flash-time macros from `shared/scripts/get_time.py`

PlatformIO runs `shared/scripts/get_time.py` as a pre-script. It appends these
`CPPDEFINES` at build time:

| Macro | Value source | Purpose |
|---|---|---|
| `FR2_LAST_FLASH_TIME` | CET Unix timestamp, clamped to `0xFFFFFFFF` | Timestamp of the build/flash operation. |
| `FR2_LAST_FLASH_YEAR` | CET year | Calendar year of the build/flash operation. |
| `FR2_LAST_FLASH_MONTH` | CET month, 1-12 | Calendar month of the build/flash operation. |
| `FR2_LAST_FLASH_WDAY` | Python `weekday()`, Monday = 0 | Weekday of the build/flash operation. |
| `FR2_LAST_FLASH_DAY` | CET day of month | Calendar day of the build/flash operation. |
| `FR2_LAST_FLASH_HOUR` | CET hour | Calendar hour of the build/flash operation. |
| `FR2_LAST_FLASH_MINUTE` | CET minute | Calendar minute of the build/flash operation. |

The RTC test firmware consumes these directly when setting/checking DS3231 time.
The external-flash test uses `FR2_LAST_FLASH_TIME` and `FRX_FW_VER_*` when present
for ROM metadata. A non-PlatformIO build that wants to run those tests should
generate equivalent `-D` flags before compiling.

Example shell shape:

```bash
now=$(TZ=CET date +%s)
year=$(TZ=CET date +%Y)
month=$(TZ=CET date +%-m)
day=$(TZ=CET date +%-d)
hour=$(TZ=CET date +%-H)
minute=$(TZ=CET date +%-M)
# Python weekday convention: Monday=0. GNU date %u is Monday=1.
wday=$(( $(TZ=CET date +%u) - 1 ))

CXXFLAGS="$CXXFLAGS \
  -DFR2_LAST_FLASH_TIME=$now \
  -DFR2_LAST_FLASH_YEAR=$year \
  -DFR2_LAST_FLASH_MONTH=$month \
  -DFR2_LAST_FLASH_WDAY=$wday \
  -DFR2_LAST_FLASH_DAY=$day \
  -DFR2_LAST_FLASH_HOUR=$hour \
  -DFR2_LAST_FLASH_MINUTE=$minute"
```

## Optional timing-sweep macros from `audio_timing_plot.py`

`shared/scripts/audio_timing_plot.py` is an analysis helper. When it runs sweeps,
it edits the `frx_test_audio` PlatformIO environment to add or replace:

- `AUDIO_TIMING_ENABLE`
- `AUDIO_BLOCK_SAMPLES=<n>`
- `AUDIO_PINGPONG_SAMPLES=<n>`

These are not global requirements. They are only needed if you want to reproduce
the audio timing plots outside PlatformIO. Normal audio firmware can rely on the
defaults in `lib/audio/audio_default_cfg.h` unless it intentionally overrides
block or ping-pong sizing.

## Module and hardware macros still apply

The macros above are only the repository-level extras. Non-PlatformIO builds must
still define the normal module toggles and required hardware flags documented in
[Configuration and defaults](configuration.md), for example:

- `FRX_ENABLE_MODULE_AUDIO` plus audio pins/topology for audio firmware.
- `FRX_ENABLE_MODULE_DSP_FRX` for DSP helpers.
- `FRX_ENABLE_MODULE_SYNTH` together with `FRX_ENABLE_MODULE_AUDIO` for synth.
- `FRX_ENABLE_MODULE_SDCARD` plus either SDMMC or SPI SD-card pin macros.
- `FRX_ENABLE_MODULE_WAV` together with its audio/SD-card dependencies.
- `FRX_ENABLE_MODULE_I2C` plus I2C pins for I2C and RTC firmware.
- `FRX_ENABLE_MODULE_EXT_FLASH` plus external-flash SPI pins.

The CLI-string generator `shared/scripts/jccl_macro_parser.py` does not add C/C++
preprocessor macros. It reads existing `*_jccl.h` string macros and generates
Python regex constants for pytest CLI tests.
