# SD Card

SD-card filesystem and streaming helpers. This module covers mounting, file operations, directory inspection, and block streaming suitable for field-recorder storage.

This page combines generated API reference cards with module-specific notes.

## Enable this module

```ini
build_flags =
    -DFRX_ENABLE_MODULE_SDCARD
    -DFRX_ENABLE_MODULE_UTILS
```

Dependencies: [Utils](utils.md).

The SD-card module also needs either `SDCARD_MODE_SDMMC` with SDMMC pins or `SDCARD_MODE_SPI` with SPI pins. See [Configuration and defaults](configuration.md) for build-flag defaults, required hardware flags, and the relation between `sd_init(...)` and `sd_init_default()`.

## Types

::: api sdcard.sd_stream_direction_t

<!-- Add handwritten notes here. -->

## Functions

::: api sdcard.sd_append

<!-- Add handwritten notes here. -->

::: api sdcard.sd_append_txt

<!-- Add handwritten notes here. -->

::: api sdcard.sd_cat

<!-- Add handwritten notes here. -->

::: api sdcard.sd_create_file

<!-- Add handwritten notes here. -->

::: api sdcard.sd_delete_file

<!-- Add handwritten notes here. -->

::: api sdcard.sd_file_exists

<!-- Add handwritten notes here. -->

::: api sdcard.sd_get_free_kbytes

<!-- Add handwritten notes here. -->

::: api sdcard.sd_get_unique_fname

<!-- Add handwritten notes here. -->

::: api sdcard.sd_init

<!-- Add handwritten notes here. -->

::: api sdcard.sd_init_default

<!-- Add handwritten notes here. -->

::: api sdcard.sd_is_mounted

<!-- Add handwritten notes here. -->

::: api sdcard.sd_job

<!-- Add handwritten notes here. -->

::: api sdcard.sd_ls

<!-- Add handwritten notes here. -->

::: api sdcard.sd_mk

<!-- Add handwritten notes here. -->

::: api sdcard.sd_mnt

<!-- Add handwritten notes here. -->

::: api sdcard.sd_read

<!-- Add handwritten notes here. -->

::: api sdcard.sd_read_txt

<!-- Add handwritten notes here. -->

::: api sdcard.sd_rm

<!-- Add handwritten notes here. -->

::: api sdcard.sd_stream_close

<!-- Add handwritten notes here. -->

::: api sdcard.sd_stream_in

<!-- Add handwritten notes here. -->

::: api sdcard.sd_stream_open

<!-- Add handwritten notes here. -->

::: api sdcard.sd_stream_out

<!-- Add handwritten notes here. -->

::: api sdcard.sd_stream_read_open

<!-- Add handwritten notes here. -->

::: api sdcard.sd_stream_write_open

<!-- Add handwritten notes here. -->

::: api sdcard.sd_unmnt

<!-- Add handwritten notes here. -->

::: api sdcard.sd_write

<!-- Add handwritten notes here. -->

::: api sdcard.sd_write_txt

<!-- Add handwritten notes here. -->
