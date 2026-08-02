# I2C

I2C base utilities for ESP32 firmware. This module owns bus initialization, serialized transfers, and device scanning.

This page combines generated API reference cards with module-specific notes.

!!! tip "Quickstart"
    Enable the module and define the I2C bus/pins in your PlatformIO environment. Replace these example values with your board routing.

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_I2C
        -DI2C_BASE_SCL=21
        -DI2C_BASE_SDA=20
        -DI2C_BASE_NUM=0
    ```

    Then initialize the default I2C bus from firmware code:

    ```cpp
    #include "i2c_base.h"

    void setup() {
        i2c_base_init_default();
    }
    ```

    See [Configuration and defaults](configuration.md) for the full build-flag/defaults model.

## Functions

Public functions for bus initialization, serialized transmit/receive operations, device scanning, and the scan job.

::: api i2c.i2c_base_scan_bus

<!-- Add handwritten notes here. -->

::: api i2c.i2c_base_scan_job

<!-- Add handwritten notes here. -->
## I2C Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_I2C` | Includes the I2C module in the build. |
| `I2C_BASE_SCL`, `I2C_BASE_SDA` | Required I2C bus pins. |
| `I2C_BASE_NUM` | Required ESP-IDF I2C bus number. |
| `I2C_BASE_SPEED` | Default bus speed used by `i2c_base_init_default()`. |
| `I2C_BASE_BUS_TXRX_TIMEOUT` | Default transmit/receive timeout. |
| `I2C_BASE_BUS_LOCK_TIMEOUT` | Default bus-lock timeout. |

