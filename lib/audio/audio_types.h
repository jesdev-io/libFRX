#ifndef _AUDIO_TYPES_H_
#define _AUDIO_TYPES_H_

#if defined(FRX_ENABLE_MODULE_AUDIO) || defined(FRX_ENABLE_MODULE_DSP_FRX)

#include <stdint.h>
#include "audio_io_arch.h"

#if AUDIO_MAX_NUM_CH>4
#error "Hardware platform only supports up to 4 channels"
#endif

#if AUDIO_MAX_NUM_CH<1
#error "Audio/DSP types need at least one audio channel to be active!"
#endif

typedef int32_t audio_sample_base_t;
typedef float audio_val_base_t;

/// @brief Sample description in time. All channels run in parallel.
/// @note Amount of possible channels set by `AUDIO_MAX_NUM_CH`, but
///       the runtime can use less than that if wanted.
typedef union {
    struct {
        audio_sample_base_t ch1;
        #if AUDIO_MAX_NUM_CH > 1
        audio_sample_base_t ch2;
        #endif
        #if AUDIO_MAX_NUM_CH > 2
        audio_sample_base_t ch3;
        #endif
        #if AUDIO_MAX_NUM_CH > 3
        audio_sample_base_t ch4;
        #endif
    }_chx;
    audio_sample_base_t ch[AUDIO_MAX_NUM_CH];
}audio_sample_t;

/// @brief Arbitrary DSP value description in time. All channels run in parallel.
/// @note Amount of possible channels set by `AUDIO_MAX_NUM_CH`, but
///       the runtime can use less than that if wanted.
typedef union {
    struct {
        audio_val_base_t ch1;
        #if AUDIO_MAX_NUM_CH > 1
        audio_val_base_t ch2;
        #endif
        #if AUDIO_MAX_NUM_CH > 2
        audio_val_base_t ch3;
        #endif
        #if AUDIO_MAX_NUM_CH > 3
        audio_val_base_t ch4;
        #endif
    }_chx;
    audio_val_base_t ch[AUDIO_MAX_NUM_CH];
}audio_val_t;

/// @brief Audio data IO struct. Represents the audio callback interface and reusable block-DSP data seam.
/// @note If only input or only output is configured, the other pointer is NULL.
typedef struct{
    audio_sample_t* in;
    audio_sample_t* out;
    uint32_t len;
    uint8_t nch;
}audio_io_t;

/// @brief Callback signature for audio blocks and compatible block-DSP users.
/// @param iobuf Shared block IO buffer.
typedef void (*audio_cb_t)(audio_io_t* iobuf);

#endif // defined(FRX_ENABLE_MODULE_AUDIO) || defined(FRX_ENABLE_MODULE_DSP_FRX)

#endif // _AUDIO_TYPES_H_
