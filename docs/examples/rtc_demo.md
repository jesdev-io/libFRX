# RTC demo application

Source: `src/rtc_demo.cpp`

This is a `jescore` compatibility-layer demo for the DS3231 RTC module. The firmware initializes `jescore`, initializes the RTC module with its default I2C bus configuration, and then waits for CLI calls.

The firmware entry point is deliberately small:

```cpp
void setup(){
    jes_init();
    i2c_rtc_ds3231_init_default();
}
```

`loop()` does not run application logic. Control happens through the RTC `jescore` job, which can be called from `jescorecli`.

## Required configuration

The demo uses `i2c_rtc_ds3231_init_default()`, so the build environment must enable the RTC module, enable its I2C dependency, and define the I2C bus pins and bus number.

At minimum, an equivalent firmware needs:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_I2C
    -DFRX_ENABLE_MODULE_RTC_DS3231
    -DI2C_BASE_SCL=21
    -DI2C_BASE_SDA=20
    -DI2C_BASE_NUM=0
```

See [Configuration and defaults](../reference/configuration.md) for the general build-flag model.

## CLI calls

Install `jescorecli` from <https://github.com/jesdev-io/jescorecli> and connect to the device.

Available RTC calls:

| Call | Effect |
|---|---|
| `rtc help` | Print available RTC commands. |
| `rtc time` | Print the current RTC time as `YYYY-MM-DD HH:MM:SS`. |
| `rtc temp` | Print the DS3231 temperature in Celsius. |

## What this example proves

This demo shows the intended split between reusable library code and firmware entry point:

- `lib/i2c/i2c_rtc_ds3231.*` owns DS3231 register access, I2C bus setup, date/time conversion, temperature reads, and CLI control;
- `src/rtc_demo.cpp` only chooses default initialization and exposes the module's `jescore` job.
