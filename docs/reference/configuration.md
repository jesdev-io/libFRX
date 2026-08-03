# Configuration and defaults

`libFRX` is configured at compile time. A firmware enables the modules it needs
and supplies board-specific pins, bus modes, and topology through build flags.
This keeps the reusable driver code in `lib/` independent and focused on the concrete hardware instance running `libFRX`.

!!! info "Compile time vs run time"
    **Compile time** configurations determine what is being baked into the firmware. Anything missing here can't be magically added in the program. **Run time** can chose what kind of features it picks from anything that was actually compiled by the compile time configuration. An example: If 2 audio channels were configured at compile time, then routing 4 of them will be impossible; just using 1 is of course up to the run time.

## Enable modules

Each optional module is guarded by an enable macro:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_AUDIO
    -DFRX_ENABLE_MODULE_SDCARD
    -DFRX_ENABLE_MODULE_WAV
```

A disabled module contributes no public declarations or implementation code to
the build. This lets small firmware variants include only the tools they use.

## Required hardware flags

Some settings cannot have safe library defaults because every board routes pins
differently. Those settings intentionally produce compile-time errors until the
consuming firmware defines them.

Example audio flags:

```ini
build_flags =
    -DAUDIO_PIN_I2S_BCLK=15
    -DAUDIO_PIN_I2S_WS=17
    -DAUDIO_PIN_I2S_IN_A=16
    -DAUDIO_PIN_I2S_OUT_A=6
```

Example I2C flags:

```ini
build_flags =
    -DI2C_BASE_SCL=21
    -DI2C_BASE_SDA=20
    -DI2C_BASE_NUM=0
```

Example SDMMC flags:

```ini
build_flags =
    -DSDCARD_MODE_SDMMC
    -DSDCARD_SDMMC_PIN_CLK=36
    -DSDCARD_SDMMC_PIN_CMD=35
    -DSDCARD_SDMMC_PIN_D0=37
    -DSDCARD_SDMMC_PIN_D1=38
    -DSDCARD_SDMMC_PIN_D2=45
    -DSDCARD_SDMMC_PIN_D3=39
```

## Default configuration headers

Complex modules keep compile-time defaults in a dedicated
`<module>_default_cfg.h` file. These defaults are fallback values which get activated if not specified. A build flag with the same macro name overrides the default.

Examples:

- `lib/audio/audio_default_cfg.h`
- `lib/i2c/i2c_base_default_cfg.h`
- `lib/i2c/i2c_rtc_ds3231_default_cfg.h`
- `lib/sdcard/sdcard_default_cfg.h`

Typical contents include:

- `jescore` job memory sizes
- buffer sizes
- default sample rates or bit depths
- timeout values
- derived configuration macros
- compile-time validation checks
- default naming schemes

## Runtime init vs default init

Most configurable modules expose two initialization paths:

- `<module>_init(...)` takes explicit runtime arguments from firmware code.
- `<module>_init_default()` consumes the compile-time values from build flags and default configuration headers.

This means a firmware can choose between runtime configuration and build-flag configuration. If a board variant should always boot with the same pins, topology, sample rate, or bus mode, define those values in `build_flags` and call `_init_default()`. If firmware needs to choose settings dynamically, call the explicit `_init(...)` routine and pass the desired values directly.

If a build flag is not provided, the corresponding `<module>_default_cfg.h` fallback is used where a safe fallback exists. Required hardware settings still fail at compile time until the consuming firmware defines them.

!!! question "Why have 2 configurations?"
    The philosophy here is that some API accesses are semantically bound by hardware. Hardware is usually fixed, so it should have a fixed feature interface. It's no good to have a software call enabling a module that the hardware does not even provide. The configuration mimics that: Compile time config for fixed hardware, run time config for dynamic setting of that hardware where possible.

## CLI string headers

modules with `jescore` CLI commands keep job names, command names, and output
messages in `<module>_jccl.h`. This keeps the implementation free of hardcoded
command strings and lets CLI tests derive expected strings from one source.

Examples:

- `lib/audio/audio_jccl.h`
- `lib/ext_flash/ext_flash_jccl.h`
- `lib/i2c/i2c_base_jccl.h`
- `lib/i2c/i2c_rtc_ds3231_jccl.h`
- `lib/sdcard/sdcard_jccl.h`

See the example CLI call sections for why these shared string headers matter in
practice: [Audio demo](../examples/audio_demo.md#cli-calls), [External flash demo](../examples/ext_flash_demo.md#cli-calls),
[I2C demo](../examples/i2c_demo.md#cli-calls), [RTC demo](../examples/rtc_demo.md#cli-calls), and
[SD card demo](../examples/sdcard_demo.md#cli-calls).

## Intended configuration flow

A project using `libFRX` should:

1. enable only the modules it uses;
2. define required board-specific hardware flags;
3. override default buffer, timeout, or format macros only when needed;
4. include the relevant module headers from firmware code;
5. initialize modules from the firmware entry point in `src/` or an application
   repository.
