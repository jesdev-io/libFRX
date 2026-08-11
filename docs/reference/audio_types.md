# Shared audio sample/value types

`lib/audio/audio_types.h` defines the shared in-memory data shapes used at the
boundary between audio callbacks and DSP-style processing:

- `audio_sample_t`
- `audio_val_t`
- `audio_io_t`
- `audio_cb_t`

This header is **not** a user-enabled module. It is compiled when at least one
module that consumes these data shapes is enabled. Today that means
`FRX_ENABLE_MODULE_AUDIO` or `FRX_ENABLE_MODULE_DSP_FRX`.

## Consumers

| Consumer module | How it consumes the types | Required build macro path |
|---|---|---|
| `audio` | Owns the sampler callback seam. `audio_io_t` carries input/output `audio_sample_t` blocks into the user callback. | Define `FRX_ENABLE_MODULE_AUDIO`; `audio.h` includes `audio_types.h`. |
| `dsp_frx` | Operates on `audio_sample_t` arrays and returns `audio_val_t` values for cleanup, mean-square, dBFS, and rolling-average helpers. | Define `FRX_ENABLE_MODULE_DSP_FRX`; `dsp_frx.h` includes `audio_types.h` without requiring the audio sampler. |
| `synth` | Writes generated samples into `audio_sample_t` buffers and is normally used from an audio output callback. | Define `FRX_ENABLE_MODULE_SYNTH` **and** its existing dependency `FRX_ENABLE_MODULE_AUDIO`; `synth.h` includes `audio.h`, which includes `audio_types.h`. |

`wav` currently stores/transfers raw sample bytes through `const void*` API
parameters. It still has an audio-module dependency for recorder-style builds,
but it does not directly expose the shared audio type names in its public API.

## Why this is split from `audio.h`

DSP helpers do not need I2S pins, DMA queues, audio control jobs, or sampler
lifecycle functions. They only need the sample/value shape. Keeping these types
in a support header lets firmware enable DSP FRX for in-memory data processing
without pulling in the hardware-facing audio module.

Synth is different: it deals in concrete audio output buffers and is designed to
be driven from the audio path, so it keeps the already-existing explicit audio
module dependency.

## API cards

::: api audio.audio_sample_t

::: api audio.audio_val_t

::: api audio.audio_io_t

::: api audio.audio_cb_t
