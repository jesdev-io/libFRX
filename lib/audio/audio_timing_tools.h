#ifndef _AUDIO_TIMING_TOOLS_H_
#define _AUDIO_TIMING_TOOLS_H_

#ifdef FRX_ENABLE_MODULE_AUDIO

#include "audio_default_cfg.h"

#ifdef AUDIO_TIMING_ENABLE

#include <stdint.h>
#include <string.h>

/// @brief Audio sampler timing counters for diagnosing DMA/event jitter.
typedef struct{
    uint32_t i2s_evt_count[audio_i2s_bank_N];
    uint32_t i2s_rx_evt_count[audio_i2s_bank_N];
    uint32_t i2s_tx_evt_count[audio_i2s_bank_N];
    uint32_t i2s_evt_min_delta_us[audio_i2s_bank_N];
    uint32_t i2s_evt_max_delta_us[audio_i2s_bank_N];
    uint32_t i2s_evt_last_delta_us[audio_i2s_bank_N];
    uint32_t process_count;
    uint32_t process_min_delta_us;
    uint32_t process_max_delta_us;
    uint32_t process_last_delta_us;
    uint32_t process_max_runtime_us;
    uint32_t read_count;
    uint32_t read_last_runtime_us;
    uint32_t read_max_runtime_us;
    uint32_t callback_count;
    uint32_t callback_last_runtime_us;
    uint32_t callback_max_runtime_us;
    uint32_t internal_callback_count;
    uint32_t internal_callback_last_runtime_us;
    uint32_t internal_callback_max_runtime_us;
    uint32_t write_count;
    uint32_t write_last_runtime_us;
    uint32_t write_max_runtime_us;
    uint32_t read_error_count;
    uint32_t write_error_count;
}audio_timing_t;

typedef struct{
    audio_timing_t counters;
    uint64_t i2s_last_us[audio_i2s_bank_N];
    uint64_t process_last_us;
}audio_timing_state_t;

uint64_t audio_timing_now_us(void);
void audio_timing_reset_state(audio_timing_state_t* state);
void audio_timing_copy_state(audio_timing_t* dst, const audio_timing_state_t* state);
void audio_timing_mark_i2s_event(audio_timing_state_t* state, uint8_t bank, bool rx);
uint64_t audio_timing_mark_process_start(audio_timing_state_t* state);
void audio_timing_mark_process_end(audio_timing_state_t* state, uint64_t process_start_us);
void audio_timing_mark_read_runtime(audio_timing_state_t* state, uint64_t start_us);
void audio_timing_mark_callback_runtime(audio_timing_state_t* state, uint64_t start_us);
void audio_timing_mark_internal_callback_runtime(audio_timing_state_t* state, uint64_t start_us);
void audio_timing_mark_write_runtime(audio_timing_state_t* state, uint64_t start_us);
void audio_timing_mark_read_error(audio_timing_state_t* state);
void audio_timing_mark_write_error(audio_timing_state_t* state);

#endif // AUDIO_TIMING_ENABLE

#endif // FRX_ENABLE_MODULE_AUDIO
#endif // _AUDIO_TIMING_TOOLS_H_
