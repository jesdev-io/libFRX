# Synth

Signal-generation helpers for writing deterministic test tones and sweeps into audio buffers.

This page combines generated API reference cards with module-specific notes.

!!! tip "Quickstart"
    Enable synth and its audio dependency. Because synth includes the audio module, the audio module's required topology/pin flags must also be present.

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_AUDIO
        -DAUDIO_IO_IN_CH=2
        -DAUDIO_IO_OUT_CH=2
        -DAUDIO_PIN_I2S_BCLK=15
        -DAUDIO_PIN_I2S_WS=17
        -DAUDIO_PIN_I2S_IN_A=16
        -DAUDIO_PIN_I2S_OUT_A=6
        -DAUDIO_PIN_I2S_IN_B=7
        -DAUDIO_PIN_I2S_OUT_B=8
        -DFRX_ENABLE_MODULE_SYNTH
    ```

    Then initialize audio, initialize synth, install a callback that writes synth samples into the audio output buffer, and start the audio sampler:

    ```cpp
    #include "audio.h"
    #include "synth.h"

    static void synth_callback(audio_io_t* io) {
        if(!io || !io->out) return;
        synth_write(io->out, io->len);
    }

    void setup() {
        audio_init_default();
        synth_init_default();
        audio_set_callback(synth_callback);
        audio_start();
    }
    ```

    See [Configuration and defaults](configuration.md) for the full build-flag/defaults model.

## Types

Public configuration types, synth selectors, and writer signatures.

::: api synth.synth_cfg_t

<!-- Add handwritten notes here. -->

::: api synth.synth_t

<!-- Add handwritten notes here. -->

::: api synth.synth_write_t

<!-- Add handwritten notes here. -->

## Functions

Public functions for synth initialization, waveform generation, and CLI argument handling.

::: api synth.synth_init

<!-- Add handwritten notes here. -->

::: api synth.synth_init_default

<!-- Add handwritten notes here. -->

::: api synth.synth_job

<!-- Add handwritten notes here. -->

::: api synth.synth_job_parse_args

<!-- Add handwritten notes here. -->

::: api synth.synth_write

<!-- Add handwritten notes here. -->

::: api synth.synth_write_saw

<!-- Add handwritten notes here. -->

::: api synth.synth_write_sine

<!-- Add handwritten notes here. -->

::: api synth.synth_write_sine_sweep

<!-- Add handwritten notes here. -->

::: api synth.synth_write_square

<!-- Add handwritten notes here. -->
## Synth Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_SYNTH` | Includes the synth module in the build. |
| `FRX_ENABLE_MODULE_AUDIO` | Required dependency; exposes audio buffer types. |
| `SYNTH_ARGS_N` | Number of CLI arguments parsed for synth settings. |
| `SYNTH_SWEEP_DUR_S` | Default sweep duration in seconds. |
| `SYNTH_SWEEP_LEN` | Default sweep length in samples. |
| `SYNTH_SWEEP_IN_MEM` | Enables precomputed sweep storage instead of computing every sweep sample on demand. |
| `SYNTH_SERVER_JOB_NAME` | `jescore` job name. |
| `SYNTH_SERVER_JOB_MEM` | `jescore` job stack size. |
| `SYNTH_CFG_DEFAULT` | Default `synth_cfg_t` initializer consumed by `synth_init_default()`. |

