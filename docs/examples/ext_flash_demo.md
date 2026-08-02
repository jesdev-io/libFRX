# External flash demo application

Source: `src/ext_flash_demo.cpp`

This is a `jescore` compatibility-layer demo for the external SPI flash module. The firmware initializes `jescore`, initializes the configured W25Qxx-style flash device, and exposes the flash diagnostic job.

The firmware entry point is deliberately small:

```cpp
void setup(){
    jes_init();
    ef_init_default();
}
```

`loop()` does not run application logic. Control happens through the external-flash `jescore` job, which can be called from `jescorecli`.

## Required configuration

The demo uses `ef_init_default()`, so the build environment must enable external flash and define the SPI host and pins.

At minimum, an equivalent firmware needs:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_EXT_FLASH
    -DEF_SPI_HOST=1
    -DEF_PIN_MISO=46
    -DEF_PIN_MOSI=11
    -DEF_PIN_SCLK=12
    -DEF_PIN_CS=14
```

See [Configuration and defaults](../reference/configuration.md) for the general build-flag model.

## CLI calls

Install `jescorecli` from <https://github.com/jesdev-io/jescorecli> and connect to the device.

Available external-flash calls:

| Call | Effect |
|---|---|
| `ef rom` | Display external flash JEDEC/PID and ROM metadata. |

## What this example proves

This demo shows the intended split between reusable library code and firmware entry point:

- `lib/ext_flash/` owns SPI setup, flash commands, ROM/settings helpers, and diagnostic output;
- `src/ext_flash_demo.cpp` only chooses default initialization and exposes the module's `jescore` job.
