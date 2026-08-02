# I2C demo application

Source: `src/i2c_demo.cpp`

This is a `jescore` compatibility-layer demo for the I2C base module. The firmware initializes `jescore`, initializes the configured I2C bus with default build-flag values, and exposes the bus scan job.

The firmware entry point is deliberately small:

```cpp
void setup(){
    jes_init();
    i2c_base_init_default();
}
```

`loop()` does not run application logic. Control happens through the I2C scan `jescore` job, which can be called from `jescorecli`.

## Required configuration

The demo uses `i2c_base_init_default()`, so the build environment must enable I2C and define the bus pins and bus number.

At minimum, an equivalent firmware needs:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_I2C
    -DI2C_BASE_SCL=21
    -DI2C_BASE_SDA=20
    -DI2C_BASE_NUM=0
```

See [Configuration and defaults](../reference/configuration.md) for the general build-flag model.

## CLI calls

Install `jescorecli` from <https://github.com/jesdev-io/jescorecli> and connect to the device.

Available I2C calls:

| Call | Effect |
|---|---|
| `i2cscan` | Scan the configured I2C bus and print found slave addresses. |

## What this example proves

This demo shows the intended split between reusable library code and firmware entry point:

- `lib/i2c/i2c_base.*` owns bus initialization, serialized transfers, and device scanning;
- `src/i2c_demo.cpp` only chooses default initialization and exposes the module's `jescore` scan job.
