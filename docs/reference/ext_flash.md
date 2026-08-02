# External Flash

External SPI flash access for W25Qxx-style devices. Use this module for persistent board metadata, settings, or raw flash pages when a variant includes compatible flash hardware.

This page combines generated API reference cards with module-specific notes.

!!! tip "Quickstart"
    Enable the module and define the external flash SPI bus/pins in your PlatformIO environment. Replace these example values with your board routing.

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_EXT_FLASH
        -DEF_SPI_HOST=SPI2_HOST
        -DEF_PIN_MISO=13
        -DEF_PIN_MOSI=11
        -DEF_PIN_SCLK=12
        -DEF_PIN_CS=10
    ```

    Then initialize the flash driver from firmware code:

    ```cpp
    #include "ext_flash.h"

    void setup() {
        ef_init_default();
    }
    ```

    See [Configuration and defaults](configuration.md) for the full build-flag/defaults model.

## Functions

Public functions for flash initialization, raw page/sector access, metadata/settings helpers, and the diagnostic job.

::: api ext_flash.ef_deinit

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_erase_sector

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_init

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_init_default

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_job

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_read

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_read_id

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_read_rom

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_read_settings

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_write_page

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_write_rom

<!-- Add handwritten notes here. -->

::: api ext_flash.ef_write_settings

<!-- Add handwritten notes here. -->
## External Flash Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_EXT_FLASH` | Includes the external flash module in the build. |
| `EF_SPI_HOST` | Required ESP-IDF SPI host/peripheral selector. |
| `EF_PIN_MISO`, `EF_PIN_MOSI`, `EF_PIN_SCLK`, `EF_PIN_CS` | Required external flash SPI pins. |
| `EF_CLK_SPEED_HZ` | SPI clock speed. |
| `EF_ROM_ADDR` | Default address for ROM-style metadata. |
| `EF_SETT_ADDR` | Default address for settings data. |
| `EF_SERVER_JOB_NAME` | `jescore` job name. |
| `EF_SERVER_JOB_MEM` | `jescore` job stack size. |

