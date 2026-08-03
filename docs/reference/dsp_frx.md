# DSP FRX

The DSP module provides multichannel sample accepting algorithms which are commonly used in signal processing, audio and field recording. 

!!! tip "Quickstart"
    Enable DSP FRX and its audio dependency. Because `dsp_frx.h` depends on `audio.h`, the audio module's required topology/pin flags must also be present.

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
        -DFRX_ENABLE_MODULE_DSP_FRX
    ```

    DSP FRX is a helper-only module and has no runtime initializer. Include it and call the helper you need:

    ```cpp
    #include "dsp_frx.h"

    float y = dsp_frx_sin_bhaskara_I(0.0f);
    ```

    See [Configuration and defaults](configuration.md) for the full build-flag/defaults model.

## Functions

Public helper functions for signal math, level conversion, rolling averages, and sample cleanup.

::: api dsp_frx.dsp_frx_cos_bhaskara_I

<!-- Add handwritten notes here. -->

::: api dsp_frx.dsp_frx_msqr_rolling_avg

<!-- Add handwritten notes here. -->

::: api dsp_frx.dsp_frx_sample_cleanup

<!-- Add handwritten notes here. -->

::: api dsp_frx.dsp_frx_samples_to_dbfs_32b

<!-- Add handwritten notes here. -->

::: api dsp_frx.dsp_frx_samples_to_dbfs_32b_from_msqr

<!-- Add handwritten notes here. -->

::: api dsp_frx.dsp_frx_samples_to_msqr_32b

<!-- Add handwritten notes here. -->

::: api dsp_frx.dsp_frx_sin_bhaskara_I

<!-- Add handwritten notes here. -->
## DSP FRX Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_DSP_FRX` | Includes the DSP FRX module in the build. |
| `FRX_ENABLE_MODULE_AUDIO` | Required dependency; exposes the audio sample/value types used here. |
| `DSP_FRX_MIC_CALIB_SPL` | Reference SPL used for SPL conversion helpers. |
| `DSP_FRX_DBFS_TO_SPL(dbfs, sens_db)` | dBFS-to-SPL conversion macro. |
| `DSP_FRX_INT24_SCALE` | Scale factor for signed 24-bit audio samples. |
| `DSP_FRX_DC_FILTER_ALPHA` | DC cleanup filter coefficient. |
| `DSP_FRX_ROLL_AVG_N` | Rolling-average window length. |

