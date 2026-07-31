# I2C

I2C base utilities for ESP32 firmware. This module owns bus initialization, serialized transfers, and device scanning.

This page combines generated API reference cards with module-specific notes.

## Enable this module

```ini
build_flags =
    -DFRX_ENABLE_MODULE_I2C
```

Dependencies: none.

See [Configuration and defaults](configuration.md) for build-flag defaults, required hardware flags, and the relation between `i2c_base_init(...)` and `i2c_base_init_default()`.

## Functions

::: api i2c.i2c_base_scan_bus

<!-- Add handwritten notes here. -->

::: api i2c.i2c_base_scan_job

<!-- Add handwritten notes here. -->
