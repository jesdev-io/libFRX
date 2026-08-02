# DS3231 RTC

DS3231 real-time-clock support built on top of the base [I2C](../i2c.md) module. The module initializes the shared I2C bus, reads/writes calendar time, reads the DS3231 temperature register, and exposes a small `jescore` CLI job.

This page is a skeleton reference page for the RTC port. It combines generated API reference cards with module-specific notes.

!!! tip "Quickstart"
    Enable the RTC module and its I2C dependency, then define the I2C bus pins in your PlatformIO environment. Replace these example values with your board routing.

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_I2C
        -DFRX_ENABLE_MODULE_RTC_DS3231
        -DI2C_BASE_SCL=21
        -DI2C_BASE_SDA=20
        -DI2C_BASE_NUM=0
    ```

    Then initialize the DS3231 from firmware code:

    ```cpp
    #include "i2c_rtc_ds3231.h"

    void setup() {
        i2c_rtc_ds3231_init_default();
    }
    ```

    See [Configuration and defaults](../configuration.md) for the full build-flag/defaults model.

## Example

See the [RTC demo application](../../examples/rtc_demo.md) for a minimal `jescore`-controlled firmware entry point.

## Functions

Public functions for RTC initialization, time read/write, temperature readout, and CLI control.

::: api i2c.i2c_rtc_ds3231_init

<!-- Add handwritten notes here. -->

::: api i2c.i2c_rtc_ds3231_init_default

<!-- Add handwritten notes here. -->

::: api i2c.i2c_rtc_ds3231_is_initialized

<!-- Add handwritten notes here. -->

::: api i2c.i2c_rtc_ds3231_get_time

`tm_year` follows standard C `struct tm` semantics: it stores years since 1900. Add `1900` when showing the full year to users.

::: api i2c.i2c_rtc_ds3231_set_time

`tm_year` follows standard C `struct tm` semantics: set 2026 as `126`, or `2026 - 1900`.

::: api i2c.i2c_rtc_ds3231_get_temp

<!-- Add handwritten notes here. -->

::: api i2c.i2c_rtc_ds3231_job

<!-- Add handwritten notes here. -->

## DS3231 RTC Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_RTC_DS3231` | Includes the DS3231 RTC module in the build. |
| `FRX_ENABLE_MODULE_I2C` | Required dependency; enables the shared I2C bus module. |
| `DS3231_I2C_ADDRESS` | DS3231 I2C slave address. Defaults to `0x68`. |
| `DS3231_JOB_MEM` | `jescore` job stack size. |
| `DS3231_YEAR_MIN`, `DS3231_YEAR_MAX` | Accepted full-year range after converting `tm_year + 1900`. |
| `DS3231_JOB_NAME` | `jescore` job name. Defaults to `rtc`. |
| `DS3231_CMD_TIME`, `DS3231_CMD_TEMP`, `DS3231_CMD_HELP` | CLI command strings. |
| `DS3231_CMDS`, `DS3231_MSG_ERROR_TIME_errnum`, `DS3231_MSG_ERROR_TEMP_errnum`, `DS3231_MSG_TIME_FORMAT`, `DS3231_MSG_TEMP_FORMAT`, `DS3231_MSG_UNKNOWN_CMD` | CLI help/status/error text. |
