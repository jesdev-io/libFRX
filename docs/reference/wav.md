# WAV

WAV header and file lifecycle helpers for recorded or played-back PCM audio.

This page combines generated API reference cards with module-specific notes.

## Enable this module

```ini
build_flags =
    -DFRX_ENABLE_MODULE_AUDIO
    -DFRX_ENABLE_MODULE_SDCARD
    -DFRX_ENABLE_MODULE_UTILS
    -DFRX_ENABLE_MODULE_WAV
```

Dependencies: [Audio](audio.md), [SD Card](sdcard.md), and [Utils](utils.md) through SD-card file support.

See [Configuration and defaults](configuration.md) for build-flag defaults and required hardware flags.

## Types

::: api wav.wav_file_t

<!-- Add handwritten notes here. -->

::: api wav.wav_hdr_t

<!-- Add handwritten notes here. -->

## Functions

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
