# External Flash

External SPI flash access for W25Qxx-style devices. Use this module for persistent board metadata, settings, or raw flash pages when a variant includes compatible flash hardware.

This page combines generated API reference cards with module-specific notes.

## Enable this module

```ini
build_flags =
    -DFRX_ENABLE_MODULE_EXT_FLASH
```

Dependencies: none.

See [Configuration and defaults](configuration.md) for build-flag defaults, required hardware flags, and the relation between `ef_init(...)` and `ef_init_default()`.

## Functions

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
