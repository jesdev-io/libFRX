#ifndef _DSP_FRX_H_
#define _DSP_FRX_H_

#ifdef FRX_ENABLE_MODULE_DSP_FRX

#include <math.h>
#include "audio.h"

// Configuration macros - can be overridden by consuming projects
// Default values are for FR1-mini compatibility

#ifndef DSP_FRX_SQUARE
#define DSP_FRX_SQUARE(x) ((x) * (x))
#endif

#ifndef DSP_FRX_MIC_SENS
#define DSP_FRX_MIC_SENS -26
#endif

#ifndef DSP_FRX_DBFS_TO_SPL
#define DSP_FRX_DBFS_TO_SPL(x) ((x) + (-1)*DSP_FRX_MIC_SENS + 94)
#endif

#ifndef DSP_FRX_INT24_SCALE
#define DSP_FRX_INT24_SCALE (1.0f / 8388608.0f)
#endif

#ifndef DSP_FRX_DC_FILTER_ALPHA
#define DSP_FRX_DC_FILTER_ALPHA (0.001)
#endif

#ifndef DSP_FRX_ROLL_AVG_N
#define DSP_FRX_ROLL_AVG_N 10
#endif

/// @brief Fast sine approximation based on Bhaskara I algorithm.
/// @param x Wrapped or unwrapped phase.
/// @return Sine approximation.
inline float dsp_frx_sin_bhaskara_I(float x);

/// @brief Convert audio samples to mean square (power) values.
/// @param data Audio sample data
/// @param len Number of samples
/// @param nch Number of active channels
/// @return Mean square values per channel
audio_val_t dsp_frx_samples_to_msqr_32b(audio_sample_t* data, uint32_t len, uint8_t nch);

/// @brief Convert audio samples to dBFS values.
/// @param data Audio sample data
/// @param len Number of samples
/// @param nch Number of active channels
/// @return dBFS values per channel
audio_val_t dsp_frx_samples_to_dbfs_32b(audio_sample_t* data, uint32_t len, uint8_t nch);

/// @brief Convert mean square values to dBFS.
/// @param msqr Mean square values per channel
/// @param nch Number of active channels
/// @return dBFS values per channel
audio_val_t dsp_frx_samples_to_dbfs_32b_from_msqr(audio_val_t msqr, uint8_t nch);

/// @brief Rolling average of mean square values.
/// @param msqr Current mean square values
/// @param nch Number of active channels
/// @return Rolling average of mean square values
audio_val_t dsp_frx_msqr_rolling_avg(audio_val_t msqr, uint8_t nch);

#endif // FRX_ENABLE_MODULE_DSP_FRX
#endif
