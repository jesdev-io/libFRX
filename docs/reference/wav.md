# WAV

WAV header and file lifecycle helpers for recorded or played-back PCM audio.

This page combines generated API reference cards with module-specific notes.

!!! tip "Quickstart"
    Enable WAV and its storage/audio dependencies, including the required audio and SD-card hardware flags. This example uses SDMMC; replace all pin numbers with your board routing.

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_AUDIO
        -DAUDIO_BANKS_CFG_DEFAULT=AUDIO_BANKS_CFG_SINGLE_STEREO_IO
        -DAUDIO_PIN_I2S_BCLK=15
        -DAUDIO_PIN_I2S_WS=17
        -DAUDIO_PIN_I2S_IN_A=16
        -DAUDIO_PIN_I2S_OUT_A=6
        -DAUDIO_PIN_I2S_IN_B=7
        -DAUDIO_PIN_I2S_OUT_B=8
        -DFRX_ENABLE_MODULE_SDCARD
        -DFRX_ENABLE_MODULE_UTILS
        -DSDCARD_MODE_SDMMC
        -DSDCARD_SDMMC_PIN_CLK=36
        -DSDCARD_SDMMC_PIN_CMD=35
        -DSDCARD_SDMMC_PIN_D0=37
        -DSDCARD_SDMMC_PIN_D1=38
        -DSDCARD_SDMMC_PIN_D2=45
        -DSDCARD_SDMMC_PIN_D3=39
        -DFRX_ENABLE_MODULE_WAV
    ```

    WAV has no standalone runtime initializer. Initialize/mount SD-card support, then open a WAV file:

    ```cpp
    #include "sdcard.h"
    #include "wav.h"

    void setup() {
        sd_init_default();
        sd_mnt();

        wav_file_t wav;
        wav_open_for_write(&wav, "/sdcard/test.wav", 2, 48000, 16);
        wav_close_for_write(&wav);
    }
    ```

    If your board uses SPI SD-card mode instead, replace the SDMMC flags with `-DSDCARD_MODE_SPI` plus `SDCARD_SPI_PIN_CS`, `SDCARD_SPI_PIN_MOSI`, `SDCARD_SPI_PIN_MISO`, and `SDCARD_SPI_PIN_SCK`.

    See [Configuration and defaults](configuration.md) for the full build-flag/defaults model.

## Types

Public header and file-context structs used by WAV read/write helpers.

::: api wav.wav_file_t

<!-- Add handwritten notes here. -->

::: api wav.wav_hdr_t

<!-- Add handwritten notes here. -->

## Functions

Public functions for WAV header creation, file open/close lifecycle, and sample transfer.

::: api wav.wav_close_for_read

<!-- Add handwritten notes here. -->

::: api wav.wav_close_for_write

<!-- Add handwritten notes here. -->

::: api wav.wav_create_header

<!-- Add handwritten notes here. -->

::: api wav.wav_open_for_read

<!-- Add handwritten notes here. -->

::: api wav.wav_open_for_write

<!-- Add handwritten notes here. -->

::: api wav.wav_read_samples

<!-- Add handwritten notes here. -->

::: api wav.wav_update_header

<!-- Add handwritten notes here. -->

::: api wav.wav_write_samples

<!-- Add handwritten notes here. -->
## WAV Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_WAV` | Includes the WAV module in the build. |
| `FRX_ENABLE_MODULE_AUDIO`, `FRX_ENABLE_MODULE_SDCARD`, `FRX_ENABLE_MODULE_UTILS` | Required dependency modules. |
| `WAV_HEADER_SIZE` | PCM WAV header byte size before padding. |
| `WAV_HEADER_PAD` | Padding bytes added after the WAV header. |
| `WAV_HEADER_SIZE_TOTAL` | Total padded header size. |
| `WAV_FN_LEN` | Filename buffer length in `wav_file_t`. |

