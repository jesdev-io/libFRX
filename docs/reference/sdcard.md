# SD Card

SD-card filesystem and streaming helpers. This module covers mounting, file operations, directory inspection, and block streaming suitable for field-recorder storage.

This page combines generated API reference cards with module-specific notes.

!!! tip "Quickstart"
    Enable SD card and utils, select a bus mode, and define the matching pins in your PlatformIO environment. This SDMMC example uses the same required flags as `sd_init_default()`; replace the pin numbers with your board routing.

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_SDCARD
        -DFRX_ENABLE_MODULE_UTILS
        -DSDCARD_MODE_SDMMC
        -DSDCARD_SDMMC_PIN_CLK=36
        -DSDCARD_SDMMC_PIN_CMD=35
        -DSDCARD_SDMMC_PIN_D0=37
        -DSDCARD_SDMMC_PIN_D1=38
        -DSDCARD_SDMMC_PIN_D2=45
        -DSDCARD_SDMMC_PIN_D3=39
    ```

    Then initialize and mount the card from firmware code:

    ```cpp
    #include "sdcard.h"

    void setup() {
        sd_init_default();
        sd_mnt();
    }
    ```

    If your board uses SPI mode instead, use `-DSDCARD_MODE_SPI` and define `SDCARD_SPI_PIN_CS`, `SDCARD_SPI_PIN_MOSI`, `SDCARD_SPI_PIN_MISO`, and `SDCARD_SPI_PIN_SCK` instead of the SDMMC pins.

    See [Configuration and defaults](configuration.md) for the full build-flag/defaults model.

## Example

See the [SD-card demo application](../examples/sdcard_demo.md) for a minimal `jescore`-controlled firmware entry point.

## Types

Public enums and descriptors used by SD-card streaming helpers.

::: api sdcard.sd_stream_direction_t

<!-- Add handwritten notes here. -->

## Functions

Public functions for initialization, mount state, file operations, directory inspection, streaming, and CLI control.

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
## SD Card Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_SDCARD` | Includes the SD-card module in the build. |
| `FRX_ENABLE_MODULE_UTILS` | Required dependency for SD-card helpers. |
| `SDCARD_MODE_SDMMC`, `SDCARD_MODE_SPI` | Select exactly one SD-card bus mode. |
| `SDCARD_SDMMC_PIN_CLK`, `SDCARD_SDMMC_PIN_CMD`, `SDCARD_SDMMC_PIN_D0`, `SDCARD_SDMMC_PIN_D1`, `SDCARD_SDMMC_PIN_D2`, `SDCARD_SDMMC_PIN_D3` | Required pins for SDMMC mode. |
| `SDCARD_SPI_PIN_CS`, `SDCARD_SPI_PIN_MOSI`, `SDCARD_SPI_PIN_MISO`, `SDCARD_SPI_PIN_SCK` | Required pins for SPI mode. |
| `SDCARD_BASE_PATH` | VFS mount base path. |
| `SDCARD_PAGE_SIZE_BYTE` | SD-card page/block byte size used by helpers. |
| `SDCARD_MAX_FILES_DEFAULT` | Default max open files for `sd_init_default()`. |
| `SDCARD_MAX_FREQ_BUS_DEFAULT` | Default bus frequency for `sd_init_default()`. |
| `SDCARD_SD_STREAM_POOL_SIZE` | Streaming job memory/pool size. |
| `SDCARD_LS_MAX_CHAR`, `SDCARD_LS_MIN_CHAR`, `SDCARD_LS_MAX_ENTRIES`, `SDCARD_LS_ENTRY_MAX_LEN`, `SDCARD_LS_ENTRY_SEPARATOR` | Directory-list output sizing/formatting. |
| `SDCARD_CAT_MAX_CHAR`, `SDCARD_PATH_MAX_CHAR` | File-print and path buffer sizing. |
| `SDCARD_JOB_TIMEOUT` | CLI/job operation timeout. |
| `SDCARD_JOB_NAME`, `SDCARD_STREAMER_JOB_NAME` | `jescore` job names. |
| `SDCARD_CMD_MOUNT`, `SDCARD_CMD_UNMOUNT`, `SDCARD_CMD_HELP`, `SDCARD_CMD_LIST`, `SDCARD_CMD_READ`, `SDCARD_CMD_CREATE`, `SDCARD_CMD_REMOVE`, `SDCARD_CMD_MEMORY` | CLI command strings. |
| `SDCARD_CMDS`, `SDCARD_MSG_MOUNTED`, `SDCARD_MSG_MOUNT_FAIL`, `SDCARD_MSG_UNMOUNTED`, `SDCARD_MSG_UNMOUNT_FAIL`, `SDCARD_MSG_LS_ERROR_errnum`, `SDCARD_MSG_CAT_ERROR_USAGE`, `SDCARD_MSG_CAT_ERROR_errnum`, `SDCARD_MSG_MK_ERROR_errnum`, `SDCARD_MSG_MK_ERROR_USAGE`, `SDCARD_MSG_RM_ERROR_errnum`, `SDCARD_MSG_RM_ERROR_USAGE`, `SDCARD_MSG_MEM_FORMAT_free_tot`, `SDCARD_MSG_MEM_ERROR_errnum`, `SDCARD_MSG_UNKNOWN_CMD` | CLI help/status/error text. |

