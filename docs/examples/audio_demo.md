# Audio demo application

Source: `src/audio_demo.cpp`

This is a `jescore` compatibility-layer demo for the audio module. The firmware
initializes the default I2S audio topology, installs a feedthrough callback, and
starts the audio sampler loop.

The callback copies each input block to the corresponding output block:

```cpp
static inline void feedthrough_cb(audio_io_t* iobuf){
    if(!iobuf->in || !iobuf->out) return;
    memcpy(iobuf->out, iobuf->in, sizeof(audio_sample_t) * iobuf->len);
}
```

The firmware entry point is deliberately small:

```cpp
void setup(){
    jes_init();
    audio_init_default();
    audio_set_callback(feedthrough_cb);
    audio_start();
}
```

`loop()` does not run application logic. Control happens through `jescore` jobs,
which can be called from `jescorecli`.

## Required configuration

The demo uses `audio_init_default()`, so the build environment must enable audio
and provide a default topology and I2S pins. The repository's `frx_demo_audio`
environment does this through build flags.

At minimum, an equivalent firmware needs:

```ini
build_flags =
    -DFRX_ENABLE_MODULE_AUDIO
    -DAUDIO_BANKS_CFG_DEFAULT=AUDIO_BANKS_CFG_SINGLE_STEREO_IO
    -DAUDIO_PIN_I2S_BCLK=15
    -DAUDIO_PIN_I2S_WS=17
    -DAUDIO_PIN_I2S_IN_A=16
    -DAUDIO_PIN_I2S_OUT_A=6
```

See [Configuration and defaults](../reference/configuration.md) for the general
build-flag model.

## CLI calls

Install `jescorecli` from <https://github.com/jesdev-io/jescorecli> and connect
to the device.

Available audio control calls:

| Call | Effect |
|---|---|
| `audioctrl status` | Print whether audio is running plus sample rate, bit depth, gain, channel count, and bank count. |
| `audioctrl stop` | Stop the audio sampler loop. |
| `audioctrl restart` | Restart audio with default/current settings. |
| `audioctrl restart sr <44100\|48000\|96000>` | Restart with a sample-rate override. |
| `audioctrl restart bps <8\|16\|24\|32>` | Restart with a bit-depth override. |
| `audioctrl restart gain <0..1>` | Restart with a gain override. |
| `audioctrl vol <0..1>` | Set output gain. Values are clipped to `0..1`. |
| `audioctrl mute` | Set output gain to `0`. |

The firmware also accepts legacy dash-prefixed restart options such as `-sr`,
`-bps`, and `-gain`, but the non-dash form is preferred for CLI use.

## What this example proves

This demo shows the intended split between reusable library code and firmware
entry point:

- `lib/audio/` owns I2S setup, buffering, sync, timing, and CLI control;
- `src/audio_demo.cpp` only chooses default initialization and supplies one user
  callback.

Replace `feedthrough_cb()` with application-specific DSP to build a simple live
audio processor or field-recorder signal path.
