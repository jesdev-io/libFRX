# Synth

Signal-generation helpers for writing deterministic test tones and sweeps into audio buffers.

This page combines generated API reference cards with module-specific notes.

## Enable this module

```ini
build_flags =
    -DFRX_ENABLE_MODULE_AUDIO
    -DFRX_ENABLE_MODULE_SYNTH
```

Dependencies: [Audio](audio.md).

See [Configuration and defaults](configuration.md) for build-flag defaults and the relation between `synth_init(...)` and `synth_init_default()`.

## Types

::: api synth.synth_cfg_t

<!-- Add handwritten notes here. -->

::: api synth.synth_t

<!-- Add handwritten notes here. -->

::: api synth.synth_write_t

<!-- Add handwritten notes here. -->

## Functions

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
