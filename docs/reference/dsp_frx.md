# DSP FRX

The DSP module provides multichannel sample accepting algorithms which are commonly used in signal processing, audio and field recording. 

!!! tip "Quickstart"
    Enable DSP FRX directly. It uses the shared `audio_types.h` sample/value types,
    but it does not require the audio sampler module or any I2S topology/pin flags.

    ```ini
    build_flags =
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
| `FRX_ENABLE_MODULE_DSP_FRX` | Includes the DSP FRX module in the build and makes the shared `audio_types.h` sample/value types available. |
| `AUDIO_MAX_NUM_CH` | Optional channel count for shared audio/DSP sample/value types; defaults to 2. |
| `DSP_FRX_MIC_CALIB_SPL` | Reference SPL used for SPL conversion helpers. |
| `DSP_FRX_DBFS_TO_SPL(dbfs, sens_db)` | dBFS-to-SPL conversion macro. |
| `DSP_FRX_INT24_SCALE` | Scale factor for signed 24-bit audio samples. |
| `DSP_FRX_DC_FILTER_ALPHA` | DC cleanup filter coefficient. |
| `DSP_FRX_ROLL_AVG_N` | Rolling-average window length. |

